import numpy as np
import scipy.stats as sps
import seaborn as sns
import matplotlib.pyplot as plt
from collections import defaultdict
from PyBTLS.lib.BTLS_collections import _Vehicle
from .load import get_gvw_from_garage, get_vehicle_length_from_garage
__all__ = ['do_gvw_stat', 'do_axle_weight_stat', 'do_vehicle_length_stat', 'do_axle_spacing_stat', 'quick_stat', 'quick_plot', 'quick_plot_setting']


def do_gvw_stat(vehicle_list:list[_Vehicle], visualise:bool=False, plot_type:str='kde', **kwargs) -> dict[str, float]:
    """
    Get statistics from the gross vehicle weight (GVW) of the input list of vehicles.

    Parameters
    ----------
    vehicle_list : list[Vehicle]
        A list of vehicles. 
    visualise : bool, optional
        Determine whether to visualise the distribution. 
    plot_type : str, optional
        Determine the plot type ('hist' or 'kde'). 

    Keyword Arguments
    -----------------
    plot_save_path : str
        The path to save the plot. 
    figsize : tuple
        The figure size.
    xlim : tuple
        The x-axis limit.
    ylim: tuple
        The y-axis limit.

    Returns
    -------
    stat_dict : dict[str, float]
        Include keys of median, mode, max, min, 25%-, 50%- and 75%-percentile, std, coefficient of variation, kurtosis, and skewness.
    """

    gvw_list = get_gvw_from_garage(vehicle_list)
    gvw_np = np.array(gvw_list)
    
    if visualise:
        ax = quick_plot_setting(title="Gross Vehicle Weight", xlabel="GVW (kN)", ylabel="Frequency", **kwargs)
        quick_plot(gvw_np, plot_type, ax, **kwargs)

    stat_dict = quick_stat(gvw_np)
    
    return stat_dict


def do_axle_weight_stat(vehicle_list:list[_Vehicle], visualise:bool=False, plot_type:str='kde', **kwargs) -> dict[str, dict[str, float]]:
    """
    Get statistics from each of the axle weights of the input list of vehicles.

    Parameters
    ----------
    vehicle_list : list[Vehicle]
        A list of vehicles. 
    visualise : bool, optional
        Determine whether to visualise the distribution. 
    plot_type : str, optional
        Determine the plot type ('hist' or 'kde'). 

    Keyword Arguments
    -----------------
    plot_save_path : str
        The path to save the plot. 
    figsize : tuple
        The figure size.
    xlim : tuple
        The x-axis limit.
    ylim: tuple
        The y-axis limit.

    Returns
    -------
    stat_dict : dict[str, dict[str, float]]
        The first dict includes keys of each axle. The second dict includes keys of median, mode, max, min, 25%-, 50%- and 75%-percentile, std, coefficient of variation, kurtosis, and skewness.
    """

    all_axle_weight_dict = defaultdict(list)
    stat_dict_dict = {}

    for vehicle in vehicle_list:
        for i in range(vehicle.get_no_axles()):
            all_axle_weight_dict[str(i)].append(vehicle.get_axle_weight(i))

    for axle_index in all_axle_weight_dict.keys():
        # do stat and save
        axle_weight_np = np.array(all_axle_weight_dict[axle_index])
        stat_dict = quick_stat(axle_weight_np)
        stat_dict_dict["Axle-"+str(int(axle_index)+1)] = stat_dict

        # plot
        if visualise:
            ax = quick_plot_setting(title=str(int(axle_index)+1)+"-Axle Weight", xlabel="Axle Weight (kN)", ylabel="Frequency", **kwargs)
            quick_plot(axle_weight_np, plot_type, ax, **kwargs)

    return stat_dict_dict


def do_vehicle_length_stat(vehicle_list:list[_Vehicle], visualise:bool=False, plot_type:str='kde', **kwargs) -> dict:
    """
    Get statistics from the vehicle length of the input list of vehicles.

    Parameters
    ----------
    vehicle_list : list[Vehicle]
        A list of vehicles. 
    visualise : bool, optional
        Determine whether to visualise the distribution. 
    plot_type : str, optional
        Determine the plot type ('hist' or 'kde'). 

    Keyword Arguments
    -----------------
    plot_save_path : str
        The path to save the plot. 
    figsize : tuple
        The figure size.
    xlim : tuple
        The x-axis limit.
    ylim: tuple
        The y-axis limit.

    Returns
    -------
    stat_dict : dict[str, float]
        Include keys of median, mode, max, min, 25%-, 50%- and 75%-percentile, std, coefficient of variation, kurtosis, and skewness.
    """

    vehicle_length_list = get_vehicle_length_from_garage(vehicle_list)
    vehicle_length_np = np.array(vehicle_length_list)
    
    if visualise:
        ax = quick_plot_setting(title="Vehicle Length", xlabel="Vehicle Length (m)", ylabel="Frequency", **kwargs)
        quick_plot(vehicle_length_np, plot_type, ax, **kwargs)
        
    stat_dict = quick_stat(vehicle_length_np)
    
    return stat_dict


def do_axle_spacing_stat(vehicle_list:list[_Vehicle], visualise:bool=False, plot_type:str='kde', **kwargs) -> dict:
    """
    Get statistics from each of the axle spacings of the input list of vehicles.

    Parameters
    ----------
    vehicle_list : list[Vehicle]
        A list of vehicles. 
    visualise : bool, optional
        Determine whether to visualise the distribution. 
    plot_type : str, optional
        Determine the plot type ('hist' or 'kde'). 

    Keyword Arguments
    -----------------
    plot_save_path : str
        The path to save the plot. 
    figsize : tuple
        The figure size.
    xlim : tuple
        The x-axis limit.
    ylim: tuple
        The y-axis limit.

    Returns
    -------
    stat_dict : dict[str, dict[str, float]]
        The first dict includes keys of each axle spacing. The second dict includes keys of median, mode, max, min, 25%-, 50%- and 75%-percentile, std, coefficient of variation, kurtosis, and skewness.
    """

    all_axle_spacing_dict = defaultdict(list)
    stat_dict_dict = {}

    for vehicle in vehicle_list:
        for i in range(vehicle.get_no_axles()-1):
            all_axle_spacing_dict[str(i)].append(vehicle.get_axle_spacing(i))
    
    for axle_index in all_axle_spacing_dict.keys():
        # do stat and save
        axle_spacing_np = np.array(all_axle_spacing_dict[axle_index])
        stat_dict = quick_stat(axle_spacing_np)
        stat_dict_dict["Axle-"+str(int(axle_index)+1)+"-"+str(int(axle_index)+2)] = stat_dict

        # plot
        if visualise:
            ax = quick_plot_setting(title=str(int(axle_index)+1)+"-"+str(int(axle_index)+2)+"-Axle Spacing", xlabel="Axle Spacing (m)", ylabel="Frequency", **kwargs)
            quick_plot(axle_spacing_np, plot_type, ax, **kwargs)
    
    return stat_dict_dict


def quick_stat(data:np.ndarray) -> dict:
    """
    Quickly get statistics from any one-dimensional numpy array.

    Parameters
    ----------
    data : np.ndarray
        Any one-dimensional numpy array.

    Returns
    -------
    stat_dict : dict[str, float]
        Include keys of median, mode, max, min, 25%-, 50%- and 75%-percentile, std, coefficient of variation, kurtosis, and skewness.
    """

    stat_dict = {}

    stat_dict['max'] = np.max(data)
    stat_dict['min'] = np.min(data)
    stat_dict['std'] = np.std(data)
    stat_dict['median'] = np.median(data)
    stat_dict['25%-percentile'] = np.percentile(data, 25)
    stat_dict['50%-percentile'] = np.percentile(data, 50)
    stat_dict['75%-percentile'] = np.percentile(data, 75)
    stat_dict['mode'] = sps.mode(np.ceil(data*10)/10.0)[0][0]
    stat_dict['cv'] = stat_dict['std'] / stat_dict['median']
    stat_dict['kurtosis'] = sps.kurtosis(data)
    stat_dict['skewness'] = sps.skew(data)

    return stat_dict


def quick_plot(data:np.ndarray, plot_type:str, ax:plt.Figure, **kwargs) -> None:
    """
    Quickly do plot via matplotlib.pyplot.

    Parameters
    ----------
    data : np.ndarray
        Any one-dimensional numpy array.
    plot_type : str
        Determine the plot type ('hist' or 'kde').
    ax : plt.Figure
        A matplotlib.pyplot figure object.

    Keyword Arguments
    -----------------
    plot_save_path : str
        The path to save the plot.

    Returns
    -------
    None
    """

    if plot_type == 'hist':
        sns.histplot(data, ax=ax)
    else:
        sns.kdeplot(data, ax=ax)
    if kwargs.get("plot_save_path") == None:
        plt.show()
    else:
        plt.savefig(kwargs.get("plot_save_path")+"_"+ax.get_title()+".png",format='png',dpi=500,pad_inches=0.1,bbox_inches='tight')


def quick_plot_setting(title:str="Data Plot", xlabel:str="X", ylabel:str="Y", **kwargs) -> plt.Figure:
    """
    Quickly do plot setting for matplotlib. 

    Parameters
    ----------
    title : str, optional
        The title of the plot.
    xlabel : str, optional
        The label of the x-axis.
    ylabel : str, optional
        The label of the y-axis.

    Keyword Arguments
    -----------------
    figsize : tuple
        The figure size.
    xlim : tuple
        The x-axis limit.
    ylim: tuple
        The y-axis limit.

    Returns
    -------
    ax : plt.Figure
        A matplotlib.pyplot figure object.
    """
    
    plt.style.use('classic')
    plt.rc('font',family='Times New Roman')

    _, ax = plt.subplots()

    if kwargs.get('figsize') != None:
        ax.figure.set_size_inches(kwargs.get('figsize'))

    ax.set_title(title)
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)

    if kwargs.get('xlim') != None:
        ax.set_xlim(kwargs.get('xlim'))
    if kwargs.get('ylim') != None:
        ax.set_ylim(kwargs.get('ylim'))

    # ax.legend()
    ax.grid(True,'both','both')

    return ax

