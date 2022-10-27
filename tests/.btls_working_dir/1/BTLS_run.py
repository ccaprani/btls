import PyBTLS;
import os;
os.chdir(".btls_working_dir/1");
btls = PyBTLS.BTLS();
btls.run("BTLSin.txt");
