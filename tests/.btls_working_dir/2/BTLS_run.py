import PyBTLS;
import os;
os.chdir(".btls_working_dir/2");
btls = PyBTLS.BTLS();
btls.run("BTLSin.txt");
