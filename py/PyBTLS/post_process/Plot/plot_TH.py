import pandas as pd
import matplotlib.pyplot as plt
__all__ = ['plot_TH']


def plot_TH(file_path:str, plot_save_path:str=None) -> None:
    """
    Plot the time history data from PyBTLS results. 
    
    Parameters
    ----------
    filename : str\n
        The name of the file to read data from.

    save_path : str, optional\n
        The path to save the plot to. If not specified, the plot will be displayed on screen.
    
    Returns
    -------
    None
    """
    plt.figure()

    # Read data
    TH_data = pd.read_csv(file_path, sep='[\t]+', skiprows=1, header=None, engine='python')
    no_load_effects = len(TH_data.columns)-2

    # Set column ids
    column_ids = ["Time (s)", "No. Trucks"]
    for i in range(no_load_effects):
        load_effect_id = "Load Effect " + str(i+1)
        column_ids.append(load_effect_id)
    TH_data.columns = column_ids

    # Plotting
    for i in range(no_load_effects):
        plt.plot(TH_data["Time (s)"], TH_data[column_ids[i+2]], label=column_ids[i+2])

    # Adding title and labels
    plt.title("Time History Data")
    plt.xlabel("Time (s)")
    plt.ylabel("Load Effects")
    plt.legend()

    # Save or display the plot
    if plot_save_path is not None:
        plt.savefig(plot_save_path+".png",format='png',dpi=500,pad_inches=0.1,bbox_inches='tight')
    else:
        plt.show()

