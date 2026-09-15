#!/usr/bin/env python3
"""Neutrino-flux reweighting and energy-distribution comparison for SND@LHC.

Overview
--------
Hadron-production models disagree about the neutrino flux reaching SND@LHC.
This module derives an energy-dependent weight w(E) that maps one model onto
another, and applies it to a simulated sample.

    w(E) = flux_fastsim(E) / flux_DPMJET(E)

where each flux is the sum of a light-hadron (pi/K) and a heavy-flavour
(charm) contribution:

    flux_DPMJET  = light_DPMJET  + charm_DPMJET      (the reference / denominator)
    flux_fastsim = light_EPOS    + charm_POWHEG      (the target  / numerator)

Multiplying every DPMJET event by w(E) therefore reproduces the fast-sim
prediction, without re-running the simulation:

    DPMJET x w(E)  ==  fast-sim (light + charm)

Two capabilities
----------------
1. ``FluxReweighter`` -- builds the ratio from the flux text files, fits a
   polynomial w(E), and writes a diagnostic plot per polynomial degree.
2. ``main()`` in "compare" mode -- reads neutrino energies straight from
   SND@LHC ROOT files (``cbmsim`` tree), applies w(E), and overlays the
   DPMJET reference with each reweighted fast-sim prediction.

Inputs
------
Flux text files: whitespace-separated, one neutrino per row, no header, with
the columns listed in ``FluxReweighter.COLS``. POWHEG files carry one extra
trailing column (``iEvent``).

ROOT files: standard SND@LHC digitised output holding the ``cbmsim`` tree with
the ``MCTrack`` branch, from which the incoming (primary) neutrino energy is
taken.

Configuration
-------------
Every input is read from YAML; nothing analysis-specific is hard-coded.

    Analysis.yaml      run settings: ROOT input, which fast-sim variants to
                       overlay, normalisation, plot styling
    Config_epos.yaml   one per fast-sim variant: flux file paths, flavour,
                       binning, fit range, cuts, weights, output label

Usage
-----
    # diagnostic ratio-fit plots for one or more reweighter configs
    python3 reweighting_function.py -m reweight -rc Config_epos.yaml

    # full comparison: fit w(E), read ROOT energies, overlay and save
    python3 reweighting_function.py -c Analysis.yaml -rc Config_epos.yaml

    # everything taken from the run YAML, no extra arguments
    python3 reweighting_function.py -c Analysis.yaml

As a library:

    from reweighting_function import FluxReweighter
    poly, chi2red = FluxReweighter("Config_epos.yaml").fit(degree=2)
    weights = poly(energies)      # w(E) evaluated per event

"""

import os
import argparse

import yaml
import numpy as np
import pandas as pd
import matplotlib
matplotlib.use("Agg")            # headless backend so plots always save
import matplotlib.pyplot as plt

# ROOT is only present in the SND environment (cvmfs). Guard the import so the
# reweighting/plotting half stays importable and testable elsewhere.
try:
    import ROOT
except ImportError:
    ROOT = None

# uproot + awkward are a pip-installable, framework-free alternative reader
# (pip install uproot awkward) that works in a plain venv without pyROOT.
try:
    import uproot
except ImportError:
    uproot = None
try:
    import awkward as ak
except ImportError:
    ak = None

try:
    from tqdm.auto import tqdm
except ImportError:                      # progress bars are optional
    def tqdm(x, **kwargs):
        return x


class FluxReweighter:
    """Derive the reweighting function w(E) = fast-sim / DPMJET from flux files.

    The class histograms the four input samples in neutrino energy, forms the
    ratio bin by bin with propagated errors, and fits a polynomial to it. All
    settings come from a YAML config passed to the constructor.

    Typical use
    -----------
        rw = FluxReweighter("Config_epos.yaml")
        rw.run()                       # diagnostic plot per degree
        poly, chi2red = rw.fit(2)      # or just the deg-2 weight function

    Attributes populated by :meth:`compute_ratio`
    ---------------------------------------------
    E      : bin centres [GeV]
    w      : the ratio fast-sim / DPMJET per bin (this is w(E) sampled)
    sig    : uncertainty on the ratio per bin
    H_DPM  : summed DPMJET histogram (light + charm), s_DPM its error
    H_LC   : summed fast-sim histogram (light + charm), s_LC its error
    """

    #: PDG codes of the neutrino flavours (matched on |pdg|, so nu and nubar).
    PID = {"nue": 12, "numu": 14, "nutau": 16}

    #: Column layout of the flux text files, in order. POWHEG files append a
    #: trailing 'iEvent' column, added on the fly in :meth:`read_files`.
    COLS = [
        "Neutrino", "Parent",
        "x in m", "y in m", "z in m",
        "x angle in rad", "y angle in rad",
        "E in GeV", "weight in pb",
    ]

    # ------------------------------------------------------------------ #
    # Construction / config
    # ------------------------------------------------------------------ #
    def __init__(self, config_path="config.yaml"):
        """Load the YAML config and prepare the default (uniform) bin edges.

        Adaptive edges, if requested, are computed later in
        :meth:`compute_ratio` because they depend on the selected events.
        """
        self.cfg = self._load_config(config_path)
        self.bins_E = np.linspace(
            self.cfg["energy_min"], self.cfg["energy_max"], num=self.cfg["bins"]
        )
        # filled in by compute_ratio()
        self.E = self.w = self.sig = None
        self.H_DPM = self.s_DPM = self.H_LC = self.s_LC = None

    @staticmethod
    def _require(d, key, section):
        """Return a mandatory config value, or raise naming the missing field."""
        if d is None or key not in d or d[key] is None:
            raise ValueError(f"Missing required config field: {section}.{key}")
        return d[key]

    @staticmethod
    def _adaptive_edges(fine, dsw, dsw2, lsw, lsw2, min_neff):
        """Build variable-width bin edges with a minimum of effective entries.

        Starting from a fine uniform grid, fine bins are accumulated left to
        right and a bin is closed as soon as *both* the DPMJET and the fast-sim
        side reach

            N_eff = (sum w)^2 / sum w^2  >=  min_neff

        N_eff is the number of unweighted events that would give the same
        statistical power, so this guarantees no bin is dominated by a handful
        of large-weight events -- the usual failure mode at high energy, where
        a single charm event with a huge weight otherwise makes the ratio jump
        and its error explode.

        Parameters
        ----------
        fine : ndarray
            Edges of the fine starting grid.
        dsw, dsw2 : ndarray
            Per-fine-bin sum of weights and sum of weights squared, DPMJET side.
        lsw, lsw2 : ndarray
            The same for the fast-sim side.
        min_neff : float
            Required effective entries per output bin.

        Returns
        -------
        ndarray
            The merged bin edges (always closed at ``fine[-1]``).
        """
        edges = [fine[0]]
        adsw = adsw2 = alsw = alsw2 = 0.0
        for i in range(len(fine) - 1):
            adsw += dsw[i]; adsw2 += dsw2[i]
            alsw += lsw[i]; alsw2 += lsw2[i]
            nd = (adsw * adsw / adsw2) if adsw2 > 0 else 0.0
            nl = (alsw * alsw / alsw2) if alsw2 > 0 else 0.0
            if min(nd, nl) >= min_neff:
                edges.append(fine[i + 1]); adsw = adsw2 = alsw = alsw2 = 0.0
        if edges[-1] != fine[-1]:
            edges.append(fine[-1])
        return np.array(edges)

    def _load_config(self, yaml_path):
        """Parse the reweighter YAML into a flat dict of settings.

        Only the four input paths are mandatory; every other key has a default
        so a minimal config stays valid. See Config_epos.yaml for the meaning
        of each field.
        """
        with open(yaml_path) as f:
            raw = yaml.safe_load(f) or {}

        inp = raw.get("inputs", {})
        eb  = raw.get("energy_binning", {})
        fr  = raw.get("fit_range", {})
        cut = raw.get("cuts", {})
        out = raw.get("output", {})
        wt  = raw.get("weights", {})

        return {
            # --- input flux files (mandatory) ---
            "light_dpm":     self._require(inp, "light_dpm", "inputs"),
            "charm_dpm":     self._require(inp, "charm_dpm", "inputs"),
            "light_fastsim": self._require(inp, "light_fastsim", "inputs"),
            "charm_fastsim": self._require(inp, "charm_fastsim", "inputs"),

            # --- neutrino flavour entering the ratio ---
            "flavour": raw.get("flavour", "mu"),

            # --- histogram binning in energy ---
            "energy_min": float(eb.get("min", 0.0)),
            "energy_max": float(eb.get("max", 4000.0)),
            "bins":       int(eb.get("bins", 51)),        # uniform mode: n edges
            "bin_mode":   eb.get("mode", "uniform"),      # "uniform" | "adaptive"
            "fine_bins":  int(eb.get("fine_bins", 400)),  # adaptive: start grid
            "min_neff":   float(eb.get("min_neff", 10.0)),# adaptive: entries/bin

            # --- energy window the polynomial is fitted over ---
            "fit_min": float(fr.get("min", 0.0)),
            "fit_max": float(fr.get("max", 4000.0)),

            # --- one diagnostic plot per degree ---
            "degrees": [int(d) for d in raw.get("polynomial_degrees", [1, 2, 3])],

            # --- quality gates on which bins enter the fit ---
            # Relative-error cap: drops bins whose sigma/ratio is too large.
            # This is the unbiased gate -- prefer it over the ratio window.
            "rel_err_max": float(raw.get("rel_err_max", 3.0)),
            # Optional window on the ratio VALUE. Keep it symmetric about the
            # data: a one-sided cap removes up-fluctuations only and biases the
            # fit low. None disables that side.
            "ratio_min": (None if raw.get("ratio_min") is None
                          else float(raw.get("ratio_min"))),
            "ratio_max": (None if raw.get("ratio_max") is None
                          else float(raw.get("ratio_max"))),

            # --- Selection cut ---
            "cut_energy_min": float(cut.get("energy_min", 10.0)),
            "cut_x_col":      cut.get("x_column", "x in m"),
            "cut_y_col":      cut.get("y_column", "y in m"),
            "cut_x_min":      float(cut.get("x_min", -46.0)),
            "cut_x_max":      float(cut.get("x_max",  -7.0)),
            "cut_y_min":      float(cut.get("y_min",  15.0)),
            "cut_y_max":      float(cut.get("y_max",  54.0)),
            # x/y are multiplied by this before the bounds are applied, so cm
            # bounds can be used with metre columns (unit_scale: 100).
            "cut_scale":      float(cut.get("unit_scale", 1.0)),
            # Set false when the inputs already passed the cut-based analysis.
            "cut_enabled":    bool(cut.get("enabled", True)),

            # --- whether 'weight in pb' is applied when histogramming ---
            # Both default to true. Disabling one side only puts numerator and
            # denominator on different scales and breaks the ratio's meaning.
            "weight_dpm":     bool(wt.get("dpm", True)),
            "weight_fastsim": bool(wt.get("fastsim", True)),

            # --- output ---
            "outpath": out.get("path", "./output"),
            "label":   out.get("label", "EPOS+POWHEG"),
        }

    # ------------------------------------------------------------------ #
    # Data helpers
    # ------------------------------------------------------------------ #
    def read_files(self, path):
        """Read one flux text file into a DataFrame with named columns.

        POWHEG samples carry an extra trailing ``iEvent`` column; this is
        detected from the file name so the column names line up either way.
        """
        cols = self.COLS + ["iEvent"] if "powheg" in path.lower() else self.COLS
        return pd.read_csv(path, sep=r"\s+", header=None, names=cols)

    def base_cut(self, df):
        """Boolean mask for the fiducial (detector acceptance) selection.

        Applies a minimum energy and an x/y box, with the x/y columns scaled by
        ``cuts.unit_scale`` first. Returns all-true when ``cuts.enabled`` is
        false, i.e. when the inputs already went through the cut-based analysis.
        """
        c = self.cfg
        if not c["cut_enabled"]:
            return pd.Series(True, index=df.index)
        xs = df[c["cut_x_col"]] * c["cut_scale"]
        ys = df[c["cut_y_col"]] * c["cut_scale"]
        return (
            (df["E in GeV"] > c["cut_energy_min"]) &
            xs.between(c["cut_x_min"], c["cut_x_max"]) &
            ys.between(c["cut_y_min"], c["cut_y_max"])
        )

    def _report_selection(self, name, df, pid):
        """Print how many rows survive each selection stage for one file.

        Printed before the ratio is built. If a stage removes everything the
        actual x/y data ranges are shown next to the cut bounds, which makes a
        unit mismatch (cut in cm, data in metres) immediately obvious.

        Returns the number of surviving rows.
        """
        c = self.cfg
        fl = (df["Neutrino"].abs() == pid)
        if not c["cut_enabled"]:
            print(f"  [{name:13s}] N={len(df):>7} | flavour={int(fl.sum()):>7} "
                  f"| cuts disabled")
            return int(fl.sum())
        e  = fl & (df["E in GeV"] > c["cut_energy_min"])
        xs = df[c["cut_x_col"]] * c["cut_scale"]
        ys = df[c["cut_y_col"]] * c["cut_scale"]
        x  = e & xs.between(c["cut_x_min"], c["cut_x_max"])
        y  = x & ys.between(c["cut_y_min"], c["cut_y_max"])
        n = int(y.sum())
        print(f"  [{name:13s}] N={len(df):>7} | flavour={int(fl.sum()):>7} "
              f"| +E>{c['cut_energy_min']:g}={int(e.sum()):>7} "
              f"| +x={int(x.sum()):>7} | +y={n:>7}")
        if n == 0:
            print(f"      0 survive -> {c['cut_x_col']}*{c['cut_scale']:g} range "
                  f"[{xs.min():.3g}, {xs.max():.3g}] vs cut [{c['cut_x_min']}, {c['cut_x_max']}]; "
                  f"{c['cut_y_col']}*{c['cut_scale']:g} range "
                  f"[{ys.min():.3g}, {ys.max():.3g}] vs cut [{c['cut_y_min']}, {c['cut_y_max']}]")
        return n

    def _hist(self, df, mask, use_weight=True):
        """Weighted energy histogram of the selected rows, with its error.

        Each event contributes its own ``weight in pb`` (or 1.0 when
        ``use_weight`` is false). The error is the standard weighted-histogram
        one, sqrt(sum w^2) per bin.

        Returns ``(counts, sigma)``.
        """
        x = df.loc[mask, "E in GeV"]
        w = df.loc[mask, "weight in pb"] if use_weight else pd.Series(1.0, index=x.index)
        counts, _ = np.histogram(x, bins=self.bins_E, weights=w)
        var, _    = np.histogram(x, bins=self.bins_E, weights=w ** 2)
        return counts.astype(float), np.sqrt(var.astype(float))

    # ------------------------------------------------------------------ #
    # Core computation
    # ------------------------------------------------------------------ #
    def compute_ratio(self):
        """Build the fast-sim / DPMJET ratio versus energy.

        Steps: read the four files, print the selection report, optionally
        derive adaptive bin edges, histogram each sample, sum light + charm on
        each side, and divide.

        Light and charm are histogrammed separately and the histograms added,
        which is equivalent to weighting every event individually and filling
        one histogram -- each event always keeps its own ``weight in pb``. The
        errors are combined in quadrature, sqrt(s_light^2 + s_charm^2).

        Populates ``E``, ``w``, ``sig``, ``H_DPM``, ``s_DPM``, ``H_LC``,
        ``s_LC`` and returns ``self`` so calls can be chained.
        """
        pid = self.PID[self.cfg["flavour"]]

        df_ld = self.read_files(self.cfg["light_dpm"])
        df_cd = self.read_files(self.cfg["charm_dpm"])
        df_lf = self.read_files(self.cfg["light_fastsim"])
        df_cf = self.read_files(self.cfg["charm_fastsim"])

        # Selection breakdown per file; also reveals unit mismatches.
        print(f"Selection report ({self.cfg['label']}, flavour={self.cfg['flavour']}):")
        n_ld = self._report_selection("light_dpm", df_ld, pid)
        n_cd = self._report_selection("charm_dpm", df_cd, pid)
        n_lf = self._report_selection("light_fastsim", df_lf, pid)
        n_cf = self._report_selection("charm_fastsim", df_cf, pid)
        if (n_ld + n_cd) == 0 or (n_lf + n_cf) == 0:
            raise ValueError(
                "Selection leaves no events on one side of the ratio "
                f"(DPMJET={n_ld + n_cd}, fast-sim={n_lf + n_cf}). See the report "
                "above: the most common cause is the x/y fiducial cut being in "
                "different units than the data (cut in cm, 'x in m' column in "
                "metres). Fix via cuts.unit_scale (e.g. 100 for m->cm) or by "
                "putting the bounds in the data's units.")

        # Optional adaptive binning. Uses the same flavour + fiducial selection
        # as the histograms below, and overwrites self.bins_E before filling.
        if self.cfg["bin_mode"] == "adaptive":
            fine = np.linspace(self.cfg["energy_min"], self.cfg["energy_max"],
                               self.cfg["fine_bins"] + 1)

            def _fine(df, use_w):
                """Sum of w and w^2 per fine bin for one selected sample."""
                m = (df["Neutrino"].abs() == pid) & self.base_cut(df)
                E = df.loc[m, "E in GeV"].to_numpy()
                w = (df.loc[m, "weight in pb"].to_numpy()
                     if use_w else np.ones(int(m.sum())))
                sw, _  = np.histogram(E, bins=fine, weights=w)
                sw2, _ = np.histogram(E, bins=fine, weights=w ** 2)
                return sw.astype(float), sw2.astype(float)

            wd, wf = self.cfg["weight_dpm"], self.cfg["weight_fastsim"]
            aLd = _fine(df_ld, wd); aCd = _fine(df_cd, wd)
            aLf = _fine(df_lf, wf); aCf = _fine(df_cf, wf)
            self.bins_E = self._adaptive_edges(
                fine, aLd[0] + aCd[0], aLd[1] + aCd[1],
                aLf[0] + aCf[0], aLf[1] + aCf[1], self.cfg["min_neff"])
            print(f"adaptive binning -> {len(self.bins_E) - 1} bins "
                  f"(min_neff={self.cfg['min_neff']:g})")

        # Denominator: DPMJET total = light + charm.
        H_L_dpm, s_L_dpm = self._hist(df_ld, (df_ld["Neutrino"].abs() == pid) & self.base_cut(df_ld), use_weight=self.cfg["weight_dpm"])
        H_C_dpm, s_C_dpm = self._hist(df_cd, (df_cd["Neutrino"].abs() == pid) & self.base_cut(df_cd), use_weight=self.cfg["weight_dpm"])
        self.H_DPM = H_L_dpm + H_C_dpm
        self.s_DPM = np.sqrt(s_L_dpm ** 2 + s_C_dpm ** 2)

        # Numerator: fast-sim total = light (EPOS) + charm (POWHEG).
        H_L_fs, s_L_fs = self._hist(df_lf, (df_lf["Neutrino"].abs() == pid) & self.base_cut(df_lf), use_weight=self.cfg["weight_fastsim"])
        H_C_fs, s_C_fs = self._hist(df_cf, (df_cf["Neutrino"].abs() == pid) & self.base_cut(df_cf), use_weight=self.cfg["weight_fastsim"])
        self.H_LC = H_L_fs + H_C_fs
        self.s_LC = np.sqrt(s_L_fs ** 2 + s_C_fs ** 2)

        # Ratio with relative errors added in quadrature. Bins empty on either
        # side are left at zero and are excluded from the fit by _fit_mask().
        centers = 0.5 * (self.bins_E[:-1] + self.bins_E[1:])
        R  = np.zeros_like(self.H_DPM)
        sR = np.zeros_like(self.H_DPM)

        m = (self.H_DPM > 0) & (self.H_LC > 0)
        R[m] = self.H_LC[m] / self.H_DPM[m]
        rel2_num = (self.s_LC[m]  / self.H_LC[m])  ** 2
        rel2_den = (self.s_DPM[m] / self.H_DPM[m]) ** 2
        sR[m] = R[m] * np.sqrt(rel2_num + rel2_den)

        self.E, self.w, self.sig = centers, R, sR
        return self

    def _fit_mask(self, verbose=False):
        """Boolean mask selecting which ratio bins enter the polynomial fit.

        A bin is kept when it lies inside ``fit_range``, has a non-zero ratio
        and error, passes the relative-error cap, and falls inside the optional
        ratio window. With ``verbose`` the number of bins removed by the ratio
        window is printed.
        """
        c = self.cfg
        rel_err = np.zeros_like(self.w)
        nz = (self.w > 0) & (self.sig > 0)
        rel_err[nz] = self.sig[nz] / self.w[nz]

        base = (
            (self.E >= c["fit_min"]) & (self.E <= c["fit_max"]) &
            (self.w > 0) & (self.sig > 0) &
            (rel_err <= c["rel_err_max"])
        )

        rmin = -np.inf if c["ratio_min"] is None else c["ratio_min"]
        rmax =  np.inf if c["ratio_max"] is None else c["ratio_max"]
        within = (self.w >= rmin) & (self.w <= rmax)

        if verbose and (c["ratio_min"] is not None or c["ratio_max"] is not None):
            dropped = int((base & ~within).sum())
            print(f"  ratio window [{rmin:g}, {rmax:g}]: dropped {dropped} of "
                  f"{int(base.sum())} otherwise-eligible bin(s)")
        return base & within

    @staticmethod
    def fit_poly_weighted(x, y, sigma, degree=1):
        """Least-squares polynomial fit weighted by 1/sigma.

        Returns ``(poly, chi2, chi2/ndf)`` where ``poly`` is a ``np.poly1d``
        with coefficients ordered highest power first.
        """
        poly  = np.poly1d(np.polyfit(x, y, deg=degree, w=1.0 / sigma))
        chi2  = np.sum(((y - poly(x)) / sigma) ** 2)
        ndof  = max(len(x) - (degree + 1), 1)
        return poly, chi2, chi2 / ndof

    def fit(self, degree):
        """Return the reweighting polynomial w(E) for one degree.

        Computes the ratio on first use. Evaluate the result per event as
        ``poly(E)``; it is only meaningful inside ``fit_range``.

        Returns ``(poly, chi2red)``. Raises ``ValueError`` if the selection
        leaves fewer points than the degree requires.
        """
        if self.E is None:
            self.compute_ratio()
        mask = self._fit_mask(verbose=True)
        n_fit = int(mask.sum())
        if n_fit < degree + 1:
            raise ValueError(
                f"Not enough fit points ({n_fit}) for degree {degree}")
        poly, _chi2, chi2red = self.fit_poly_weighted(
            self.E[mask], self.w[mask], self.sig[mask], degree=degree
        )
        return poly, chi2red

    # ------------------------------------------------------------------ #
    # Plotting
    # ------------------------------------------------------------------ #
    @staticmethod
    def _poly_equation_text(poly, deg):
        """Format w(E) as a LaTeX string for the plot's statistics box."""
        terms = []
        for p, coef in zip(range(deg, -1, -1), poly.coefficients):
            if p == 0:
                terms.append(rf"({coef:.3e})")
            elif p == 1:
                terms.append(rf"({coef:.3e})\,E")
            else:
                terms.append(rf"({coef:.3e})\,E^{p}")
        return r"$w(E) = " + " + ".join(terms) + r"$"

    def _make_plot(self, deg, poly, chi2red, fit_mask):
        """Write the diagnostic ratio plot for one polynomial degree.

        Shows every ratio bin faintly, the subset entering the fit with error
        bars, the fitted curve inside ``fit_range`` and its extrapolation
        outside, plus a statistics box. Comparing the two point sets shows at a
        glance what the quality gates removed.

        Returns the path of the saved PDF.
        """
        c = self.cfg
        E, w, sig = self.E, self.w, self.sig
        E_fit, w_fit, sig_fit = E[fit_mask], w[fit_mask], sig[fit_mask]

        E_plot_fit = np.linspace(c["fit_min"], c["fit_max"], 300)
        E_plot_all = np.linspace(E.min(), E.max(), 400)

        fig, ax = plt.subplots(figsize=(9, 5.8))
        ax.errorbar(E, w, yerr=sig, fmt="o", ms=4, alpha=0.25, label="all data")
        ax.errorbar(E_fit, w_fit, yerr=sig_fit, fmt="o", ms=5, capsize=3, label="used in fit")
        ax.plot(E_plot_fit, poly(E_plot_fit), "-", lw=2.5, label=f"poly deg={deg} (fit range)")
        ax.plot(E_plot_all, poly(E_plot_all), "--", lw=1.8, alpha=0.6, label="poly extrapolation")

        ax.axvline(c["fit_min"], color="grey", ls=":", lw=1)
        ax.axvline(c["fit_max"], color="grey", ls=":", lw=1)
        ax.axhline(1.0, color="grey", ls="--", lw=1)   # ratio = 1: models agree

        ax.set_xlabel("Energy [GeV]")
        ax.set_ylabel("ratio")
        ax.set_title(c["label"])
        ax.grid(True, which="both", alpha=0.3)
        ax.legend(fontsize="small", loc="lower left")

        stats_text = "\n".join([
            rf"deg = {deg}",
            rf"$\chi^2$/ndf = {chi2red:.2f}",
            rf"$N_\mathrm{{points}}$ = {len(E_fit)}",
            rf"$E_\mathrm{{fit}}$: {c['fit_min']:.0f}-{c['fit_max']:.0f} GeV",
            self._poly_equation_text(poly, deg),
        ])
        ax.text(0.98, 0.95, stats_text, transform=ax.transAxes,
                ha="right", va="top", fontsize=10,
                bbox=dict(boxstyle="round", facecolor="white", alpha=0.85))

        fig.tight_layout()
        outfile = os.path.join(c["outpath"],
                               f"reweight_poly_deg{deg}_{c['flavour']}.pdf")
        fig.savefig(outfile)
        plt.close(fig)
        return outfile

    # ------------------------------------------------------------------ #
    # Driver
    # ------------------------------------------------------------------ #
    def run(self):
        """Fit every degree in ``polynomial_degrees`` and save one plot each.

        This is the "reweight" mode entry point: it is the diagnostic pass used
        to choose a degree and check the binning and quality gates. Degrees
        with too few surviving points are skipped with a message. Returns
        ``self``.
        """
        os.makedirs(self.cfg["outpath"], exist_ok=True)   # create BEFORE saving
        if self.E is None:
            self.compute_ratio()

        fit_mask = self._fit_mask(verbose=True)
        n_fit = int(fit_mask.sum())

        for deg in self.cfg["degrees"]:
            if n_fit < deg + 1:
                print(f"[skip] deg={deg}: only {n_fit} fit points (need {deg + 1})")
                continue
            poly, chi2, chi2red = self.fit_poly_weighted(
                self.E[fit_mask], self.w[fit_mask], self.sig[fit_mask], degree=deg
            )
            print("Saved:", self._make_plot(deg, poly, chi2red, fit_mask))

        print("Output dir:", self.cfg["outpath"])
        return self


# ==========================================================================
# Run-level configuration (Analysis.yaml)
# ==========================================================================

#: PDG codes used when selecting the incoming neutrino in the ROOT files.
NU_PID = {"e": 12, "mu": 14, "tau": 16}


def load_analysis_config(path):
    """Load the top-level run YAML (Analysis.yaml) into a dict."""
    with open(path) as f:
        return yaml.safe_load(f) or {}


def resolve_normalization(norm, number_files, target_luminosity=68.6,
                          luminosity_per_file=100.0):
    """Return the factor scaling the simulated sample to the target luminosity.

    The automatic value is the luminosity ratio L_data / L_MC,

        target_luminosity / (number_files * luminosity_per_file)

    with each input file representing ``luminosity_per_file`` [fb^-1] and
    ``number_files`` of them merged. Passing a number instead of ``"auto"``
    uses it verbatim; ``"auto"`` or a non-positive value uses the formula.
    """
    auto = target_luminosity / (number_files * luminosity_per_file)
    if isinstance(norm, str) and norm.strip().lower() == "auto":
        return auto
    norm = float(norm)
    return norm if norm > 0 else auto


def build_fastsim_curves(fastsim_configs, reference, degree):
    """Fit w(E) for each fast-sim variant and assemble the curves to draw.

    One ``FluxReweighter`` is run per config, so the weights come from the fit
    and are never hard-coded. The fitted coefficients and chi2/ndf are printed
    for the record, and each curve is labelled from its own config.

    Returns a list of ``(label, weight_poly | None, color, linestyle)`` with the
    unweighted DPMJET reference (``weight_poly`` None) first.
    """
    curves = [(reference.get("label", "DPMJET (reference)"),
               None,
               reference.get("color", "black"),
               reference.get("linestyle", "-"))]
    for entry in fastsim_configs:
        rw = FluxReweighter(entry["config"])
        poly, chi2red = rw.fit(degree=degree)
        label = rw.cfg["label"]
        coeffs = np.array2string(poly.coefficients, precision=4, separator=", ")
        print(f"{label}: fast-sim(light+charm)/DPMJET  deg={degree}  "
              f"coeffs(high->low)={coeffs}  chi2/ndf={chi2red:.2f}")
        curves.append((label, poly,
                       entry.get("color", "tab:blue"),
                       entry.get("linestyle", "--")))
    return curves


# ==========================================================================
# Reading energies from ROOT (SND@LHC cbmsim tree)
# ==========================================================================
def dpm_file_list(data_dir, n_files, pattern, indexed=True):
    """Build the list of ROOT files to read.

    ``indexed=True`` expects the per-job layout ``<data_dir>/<i>/<pattern>``
    for i in range(n_files); ``indexed=False`` a single merged file at
    ``<data_dir>/<pattern>``.
    """
    if indexed:
        return [os.path.join(data_dir, str(i), pattern) for i in range(n_files)]
    return [os.path.join(data_dir, pattern)]


def build_chain(files, tree_name="cbmsim"):
    """Chain the given ROOT files into one TChain (pyROOT only)."""
    if ROOT is None:
        raise RuntimeError("ROOT is not available in this environment.")
    chain = ROOT.TChain(tree_name)
    added = sum(bool(chain.Add(f)) for f in files)
    print(f"Chained {added}/{len(files)} files (tree '{tree_name}')")
    return chain


def _incoming_neutrino_energy(tree, pids):
    """Energy of the primary neutrino in the current entry, or None.

    The incoming neutrino is the MCTrack whose |pdg| matches the requested
    flavour and which has no mother (MotherId == -1).
    """
    for trk in tree.MCTrack:
        if abs(trk.GetPdgCode()) in pids and trk.GetMotherId() == -1:
            return trk.GetEnergy()
    return None


def get_energies_pyroot(files, flavour="mu", tree_name="cbmsim", n_events=None):
    """Read per-event neutrino energies [GeV] with pyROOT.

    Loops the chained ``cbmsim`` tree entry by entry. Returns a 1-D array with
    one energy per event that contains a matching primary neutrino.
    """
    pids = set(NU_PID.values()) if flavour == "all" else {NU_PID[flavour]}
    chain = build_chain(files, tree_name)
    n_total = chain.GetEntries()
    n = n_total if n_events is None else min(n_events, n_total)

    energies = []
    for i in tqdm(range(n), desc="reading neutrino energy (pyROOT)"):
        chain.GetEntry(i)
        E = _incoming_neutrino_energy(chain, pids)
        if E is not None:
            energies.append(E)
    arr = np.asarray(energies, dtype=float)
    print(f"Extracted {arr.size} neutrino energies (flavour={flavour})")
    return arr

def _find_mctrack_branches(tree):
    """Locate the MCTrack member branches whatever the exact prefix is.

    Different productions store them as ``MCTrack.fPdgCode``,
    ``MCTrack/fPdgCode`` and so on, so the names are matched by suffix.
    Returns a dict mapping the member name to the branch key found.
    """
    want = ["fPdgCode", "fMotherId", "fPx", "fPy", "fPz"]
    keys = tree.keys()
    found = {}
    for w in want:
        cands = ([k for k in keys if "MCTrack" in k and k.endswith("." + w)]
                 or [k for k in keys if "MCTrack" in k and k.endswith(w)]
                 or [k for k in keys if k.endswith(w)])
        if cands:
            found[w] = cands[0]
    return found

def get_energies_uproot(files, flavour="mu", tree_name="cbmsim", n_events=None):
    """Read per-event neutrino energies [GeV] with uproot, without ROOT.

    Selects the incoming neutrino (|pdg| matching the flavour, fMotherId == -1)
    and, neutrinos being effectively massless, takes E = |p|. Unreadable files
    are skipped with a message.
    """
    if uproot is None or ak is None:
        raise RuntimeError("uproot/awkward not available -- pip install uproot awkward")
    pids = set(NU_PID.values()) if flavour == "all" else {NU_PID[flavour]}

    out = []
    for path in tqdm(files, desc="reading neutrino energy (uproot)"):
        try:
            tree = uproot.open(f"{path}:{tree_name}")
        except Exception as exc:                      # noqa: BLE001
            print(f"  [skip] {path}: {exc}")
            continue
        br = _find_mctrack_branches(tree)
        if len(br) < 5:
            raise RuntimeError(
                "Could not find MCTrack momentum/pdg branches. Branches seen: "
                f"{tree.keys()[:40]} ... adjust _find_mctrack_branches().")
        a = tree.arrays(list(br.values()), entry_stop=n_events, library="ak")
        pdg, mom = a[br["fPdgCode"]], a[br["fMotherId"]]
        px, py, pz = a[br["fPx"]], a[br["fPy"]], a[br["fPz"]]

        is_nu = ak.zeros_like(mom, dtype=bool)
        for pid in pids:
            is_nu = is_nu | (abs(pdg) == pid)
        sel = is_nu & (mom == -1)

        E = np.sqrt(px ** 2 + py ** 2 + pz ** 2)
        first = ak.firsts(E[sel])                     # first primary nu per event
        first = first[~ak.is_none(first)]
        out.append(ak.to_numpy(first))

    arr = np.concatenate(out) if out else np.array([], dtype=float)
    print(f"Extracted {arr.size} neutrino energies (flavour={flavour})")
    return arr


def read_dpm_energies(ri, reader="auto"):
    """Read the DPMJET neutrino energies using the requested ROOT backend.

    ``ri`` is the ``root_input`` block of the run YAML. ``reader`` is "auto"
    (pyROOT when importable, else uproot), "pyroot" or "uproot".
    """
    files = dpm_file_list(ri["data_dir"], int(ri["number_files"]),
                          ri["pattern"], indexed=ri.get("indexed", True))
    tree_name = ri.get("tree_name", "cbmsim")
    flavour = ri.get("flavour", "mu")

    if reader == "auto":
        reader = "pyroot" if ROOT is not None else "uproot"

    if reader == "pyroot":
        if ROOT is None:
            raise RuntimeError("reader=pyroot but ROOT is unavailable. Source the "
                               "SND@LHC setup, or use reader: uproot "
                               "(pip install uproot awkward).")
        return get_energies_pyroot(files, flavour, tree_name)
    if reader == "uproot":
        return get_energies_uproot(files, flavour, tree_name)
    raise ValueError(f"unknown reader: {reader!r} (use auto/pyroot/uproot)")


# ==========================================================================
# Comparison plot
# ==========================================================================
def make_comparison_plot(dpm, curves, normalized_factor, outfile,
                         bins=100, hist_range=(0, 2000), title=None):
    """Overlay the DPMJET energy spectrum with each reweighted fast-sim curve.

    The same array of DPMJET energies is histogrammed once per curve. The
    reference is unweighted; every other curve applies its w(E) per event, so
    ``DPMJET x w(E)`` is the fast-sim prediction. The binning here is
    independent of the binning used to derive w(E): the weight is evaluated
    from the polynomial at each event's energy, not looked up by bin.

    The integral quoted in each legend entry is the normalised area under the
    histogram, i.e. the expected yield at the target luminosity.

    Returns a dict of ``{label: integral}``.
    """
    if title is None:
        title = "Energy Distribution: DPMJET vs fast-sim (light + charm) reweighted"

    plt.figure(figsize=(12, 7))
    integrals = {}

    for label, weight_poly, color, linestyle in curves:
        weights = None if weight_poly is None else weight_poly(dpm)

        # Histogram used for the integral only (not drawn).
        counts, edges = np.histogram(
            dpm, bins=bins, range=hist_range, weights=weights
        )
        bin_width = edges[1] - edges[0]
        integral = (normalized_factor * np.sum(counts) * bin_width
                    * (bins / (hist_range[1] - hist_range[0])))
        integrals[label] = integral

        plt.hist(dpm, bins=bins, range=hist_range, histtype="step",
                 weights=weights, color=color, linestyle=linestyle,
                 label=f"{label} (\u222bfde \u2248 {integral:.1f})")

    plt.xlabel("Energy GeV")
    plt.ylabel(f"Events / {(hist_range[1] - hist_range[0]) / bins} GeV")
    plt.title(title)
    plt.legend()
    plt.grid(alpha=0.3)
    plt.tight_layout()

    os.makedirs(os.path.dirname(os.path.abspath(outfile)), exist_ok=True)
    plt.savefig(outfile, dpi=150)
    plt.close()
    print("Saved:", outfile)

    print("\nIntegrals (sum(counts)*bin_width, normalized):")
    for label, val in integrals.items():
        print(f"{label:45s}: {val:.3f}")
    return integrals


# ==========================================================================
# Command line entry point
# ==========================================================================

#: Fallback styling when fast-sim configs are given on the command line and so
#: carry no colour/linestyle of their own.
DEFAULT_COLORS = ["tab:blue", "tab:red", "tab:green", "tab:orange", "tab:purple"]
DEFAULT_STYLES = ["--", "-.", ":"]


def fastsim_entries_from_args(paths):
    """Turn bare config paths from ``-rc`` into styled fast-sim entries."""
    return [{"config": path,
             "color": DEFAULT_COLORS[i % len(DEFAULT_COLORS)],
             "linestyle": DEFAULT_STYLES[i % len(DEFAULT_STYLES)]}
            for i, path in enumerate(paths)]


def main():
    """Parse the command line and run the requested mode.

    Modes
    -----
    reweight : fit each reweighter config and save its diagnostic plots. Needs
               no run YAML when both ``-m reweight`` and ``-rc`` are given.
    compare  : fit w(E), read the DPMJET energies from ROOT, and save the
               overlay. Needs the run YAML for the ROOT input and plot settings.

    Command-line options override the corresponding run-YAML fields; anything
    not given falls back to the YAML.
    """
    p = argparse.ArgumentParser(
        description="Flux reweighting + energy-distribution comparison (YAML driven)")
    p.add_argument("-c", "--config", default="analysis.yaml",
                   help="Top-level analysis YAML with the run settings")
    p.add_argument("-rc", "--reweight-config", nargs="+", default=None,
                   metavar="YAML",
                   help="FluxReweighter config file(s). In compare mode these are "
                        "the fast-sim variants (overrides fastsim_configs in the "
                        "run YAML); in reweight mode each is fit and plotted.")
    p.add_argument("-m", "--mode", choices=["compare", "reweight"], default=None,
                   help="Override the 'mode' set in the run YAML")
    p.add_argument("--reader", choices=["auto", "pyroot", "uproot"], default=None,
                   help="ROOT backend: auto (default), pyroot, or uproot "
                        "(pip install uproot awkward; no ROOT framework needed)")
    args = p.parse_args()

    # The run YAML is loaded lazily, so 'reweight' mode with -rc works in a
    # directory that has no Analysis.yaml at all.
    _cfg = {}

    def get_cfg():
        nonlocal _cfg
        if not _cfg:
            try:
                _cfg = load_analysis_config(args.config)
            except FileNotFoundError:
                raise SystemExit(
                    f"Run config '{args.config}' not found. It's needed for "
                    "compare mode (and to read 'mode'/'reweight_only' when not "
                    "given on the CLI). Pass -c <file>, or in reweight mode give "
                    "-m reweight with -rc <config(s)> so no run YAML is required.")
        return _cfg

    mode = args.mode or get_cfg().get("mode", "compare")

    if mode == "reweight":
        paths = args.reweight_config or [get_cfg()["reweight_only"]["config"]]
        for path in paths:
            FluxReweighter(path).run()
        return

    # --- compare mode ---
    cfg  = get_cfg()
    ri   = cfg["root_input"]
    rw   = cfg.get("reweight", {})
    plot = cfg.get("plot", {})
    degree = int(rw.get("degree", 2))

    fastsim = (fastsim_entries_from_args(args.reweight_config)
               if args.reweight_config else cfg["fastsim_configs"])

    # 1) reweighting polynomials, fitted by FluxReweighter
    curves = build_fastsim_curves(fastsim, cfg.get("reference", {}), degree=degree)

    # 2) DPMJET neutrino energies from ROOT (pyROOT or uproot)
    reader = args.reader or ri.get("reader", "auto")
    dpm = read_dpm_energies(ri, reader=reader)
    if dpm.size == 0:
        raise SystemExit("No energies extracted -- check files / branch names.")

    # 3) overlay DPMJET with each reweighted fast-sim curve
    norm = resolve_normalization(
        plot.get("normalization", "auto"),
        int(ri["number_files"]),
        target_luminosity=float(plot.get("target_luminosity", 68.6)),
        luminosity_per_file=float(plot.get("luminosity_per_file", 100.0)),
    )
    make_comparison_plot(
        dpm, curves, normalized_factor=norm,
        outfile=plot.get("outfile", "energy_comparison.pdf"),
        bins=int(plot.get("bins", 100)),
        hist_range=(0, float(plot.get("energy_max", 2000.0))),
        title=plot.get("title"),
    )


if __name__ == "__main__":
    main()