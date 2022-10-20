import pkgutil

"""
Helper function to read and parse csv file. Allows reading of relative path within the default_files directory  context
Input:
    path: relative path to the file within the default_files context
Output:
    CSV file as list
"""
def read_default_raw_file(path):
    return pkgutil.get_data(__name__, path).decode()

def read_default_file(path):
    data = read_default_raw_file(path)
    data = data.split('\n')
    return [t.split(',') for t in data]
