"""
Analysis tools for simulation results, e.g. extreme value fitting.
"""

from .extreme import DAYS_PER_YEAR, GEVFit, GPDFit, fit_gev, fit_gpd
from .plot import (
    plot_mean_residual_life,
    plot_parameter_stability,
    plot_qq,
    plot_return_level,
)

__all__ = [
    "DAYS_PER_YEAR",
    "GEVFit",
    "GPDFit",
    "fit_gev",
    "fit_gpd",
    "plot_mean_residual_life",
    "plot_parameter_stability",
    "plot_qq",
    "plot_return_level",
]
