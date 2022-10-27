#import PyBTLS
import numpy as np
import warnings
import os
import shutil
import pickle
from PyBTLS.py_module.utils._helper_class import _ListLike
from PyBTLS.py_module.parallel.wrapper import BTLSWrapper

"""
Class to run BTLS in parallel.
Will be based on either _DfBased or _ListLike in the future
"""
class BTLSParallelWrapper(_ListLike):
    def __init__(self, num_subprocess = None, BTLSWrapper = None, working_dir = ".btls_working_dir"):
        self.num_subprocess = num_subprocess
        self.working_dir = working_dir
        if (not BTLSWrapper == None):
            if not isinstance(BTLSWrapper, list):
                self._BTLSWrapper = [BTLSWrapper]
            else:
                self._BTLSWrapper = BTLSWrapper
        else:
            self._BTLSWrapper = []
    
    @property
    def BTLSWrapper(self):
        return self._BTLSWrapper
    @BTLSWrapper.setter
    def BTLSWrapper(self, BTLSWrapper):
        if not isinstance(BTLSWrapper, list):
            self._BTLSWrapper = [BTLSWrapper]
        else:
            self._BTLSWrapper = BTLSWrapper
        
    def add_BTLSWrapper(self, instance):
        if not isinstance(instance, list):
            instance = [instance]
        self._BTLSWrapper.extend(instance)
        
    def add(self, instance):
        #Alias for add_BTLS_instance
        self.add_BTLSWrapper(instance)
        
    def _create_working_dir(self):
        os.makedirs("./" + self.working_dir, exist_ok=True)
        
    def _delete_working_dir(self):
        try:
            shutil.rmtree("./" + self.working_dir)
            return True
        except :
            return False
    
    def run(self):
        #In the future, should call _select_run() instead, with the entire bridges as input
        self._create_working_dir()
        num_bridge = len(self.BTLSWrapper)
        required_n_loop = int(np.ceil(num_bridge/self.num_subprocess))
        calc_performed = 0
        for i in range(0, required_n_loop):
            calc_remaining = num_bridge - calc_performed
            subprocess = []
            for k in range(0, np.minimum(self.num_subprocess, calc_remaining)):
                offset = i*self.num_subprocess
                print("Starting Kernel " + str(k+offset))
                self.BTLSWrapper[k+offset].working_dir = self.working_dir + "/" + str(k+offset)
                subprocess.append(self.BTLSWrapper[k+offset]._begin_parallel_run())
                
            #Wait for all subprocesses to finish
            for k in range(0, np.minimum(self.num_subprocess, calc_remaining)):
                subprocess[k].wait()
                
            calc_performed += self.num_subprocess
            
        #end parallel process
        #Just do normal loop here. Should be fast enough
        for i in range(0, num_bridge):
            self.BTLSWrapper[i]._end_parallel_run()
        #self._delete_working_dir()

    @property
    def __primarykey__(self):
        return "BTLSWrapper"

    @property
    def __setitem_acceptable_type__(self):
        return BTLSWrapper

    @property
    def __str_quantifier__(self):
        return "BTLS simulation(s)"

"""
Method to import btls_parallel.obj from multiple folders. Useful when doing cluster compute.
WARNING: This method was created as a stop gap to import results for cluster computing. It WILL be deprecated and removed in the future!
Input:
    fpath: path to each subfolders for each cluster compute unit
"""
def import_btls_parallel_from_multiple_folders(fpath):
    warnings.warn("WARNING: This method was created as a stop gap to import results for cluster computing. It WILL be deprecated and removed in the future!")
    btls_parallel_list = []

    for file in fpath:
        with open(file + "/btls_parallel.obj", 'rb') as f:
            btls_parallel_list.append(pickle.load(f))

    proto_btls_parallel_obj = btls_parallel_list[0]
    for i, b in enumerate(btls_parallel_list):
        if i > 0:
            proto_btls_parallel_obj.add(b.BTLSWrapper)
    return proto_btls_parallel_obj