
r"""

"""

from PyBTLS.py_module.misc._helper_class import _DfBased

class _BtlsWrapper_DefaultOutputs():
    def __init__(self):
        self.interval_statistics = "No Output Set"
        self.vehicles = "No Output Set"
        self.block_maxima_summary = "No Output Set"

    def __str__(self):
        return super().__str__()

class OutputIntervalStatistics(_DfBased):
    def _from_txt_to_dataframe_kwargs(self, txt, *args, **kwargs):
        ...
        
class BtlsOutputBlockMaximaSummary(_DfBased):
    def _from_txt_to_dataframe_kwargs(self, txt, *args, **kwargs):
        ...

"""
class BtlsOutputIntervalStatistics(BtlsOutput):
    def _from_txt_to_core_data(self, txt):
        txt = txt.split('\n') #Split by line
        data = [text.split() for text in txt[1:-1]] #Split by column, ignore header and empty row at the end
        #Equalise the length of the data by padding the list at the end
        pad = len(max(data, key=len)) 
        data = np.array([i + [0]*(pad-len(i)) for i in data])

        base_header = [
            'id',
            'time',
            'num_events',
            'num_event_vehicles',
            'num_event_trucks',
            'min',
            'max',
            'mean',
            'std_dev',
            'variance',
            'skeweness',
            'kurtosis',
        ]

        column_type = {
            'id': int,
            'time': float,
            'num_events': int,
            'num_event_vehicles': int,
            'num_event_trucks': int,
            'min': float,
            'max': float,
            'mean': float,
            'std_dev': float,
            'variance': float,
            'skeweness': float,
            'kurtosis': float,
        }

        #Get size of data, check how many excess truck # event if there's any
        num_defined_axles = data.shape[1] - 12

        #Append the axle info
        axle_header = ['event_' + str(i+1) + '_truck' for i in range(0,num_defined_axles)]
        header = np.append(base_header, axle_header)
        axle_column_type = dict(zip(axle_header, np.array([float for i in range(0,num_defined_axles)])))
        column_type.update(axle_column_type)

        df = pd.DataFrame(data, columns = header)
        df = df.astype(column_type)

        return df
    
    def _from_core_data_to_txt(self):
        pass


"""

"""

class BtlsOutputBlockMaximaSummary(BtlsOutput):
    def _from_txt_to_core_data(self, txt):
        txt = txt.split('\n') #Split by line
        data = [re.split("\t\t|\t", t)[0:-1] for t in txt][0:-1] #Ignore empty column at the end. Ignore empty row at the end.
        #Equalise the length of the data by padding the list at the end
        pad = len(max(data, key=len)) 
        data = np.array([i + [0]*(pad-len(i)) for i in data])

        base_header = [
            "index",
            'event_1_truck',
        ]

        column_type = {
            "index": int,
            'event_1_truck': float,
        }

        #Get size of data, check how many excess truck # event if there's any
        num_defined_axles = data.shape[1] - 2

        #Append the axle info
        axle_header = ['event_' + str(i+2) + '_truck' for i in range(0,num_defined_axles)]
        header = np.append(base_header, axle_header)
        axle_column_type = dict(zip(axle_header, np.array([float for i in range(0,num_defined_axles)])))
        column_type.update(axle_column_type)

        df = pd.DataFrame(data, columns = header)
        df = df.astype(column_type)

        return df

    def _from_core_data_to_txt(self):
        pass


"""