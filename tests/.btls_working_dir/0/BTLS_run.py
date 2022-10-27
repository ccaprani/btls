import PyBTLS;
import os;
os.chdir(".btls_working_dir/0");
btls = PyBTLS.BTLS();
btls.run("BTLSin.txt");
