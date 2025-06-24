Instructions for Developers
===========================

This file presents instructions for ``PyBTLS`` developers.

.. _install_dev:

Create working repository with developer install
------------------------------------------------

1. Fork ``PyBTLS`` `GitHub repository <https://github.com/ccaprani/btls/>`_

2. Clone repo ::

	git clone <forked-repo>

3. Create new `PyBTLS` developer environment ::
	
	conda env create -f environment.yaml

4. Activate developer environment ::

	conda activate pybtls-dev

5. Change directory to pybtls fork ::

	cd path_to_forked_repo

6. Install developer version ::

	pip install -e .

7. Install depedencies ::

	conda install -c anaconda pytest
	conda install -c anaconda sphinx
	conda install -c conda-forge black
	pip install sphinx_autodoc_typehints
	pip install nbsphinx
	pip install pydata_sphinx_theme

8. Add ``PyBTLS`` as upstream ::

	git remote add upstream https://github.com/ccaprani/btls.git

.. _pr:

Develop and create pull-request (PR)
------------------------------------

1. Create new branch ::

	git checkout -b <new-branch>

2. Pull updates from PyBTLS main ::
	
	git pull upstream main 

3. Develop package

	* For new features related to the simulation, modify the C++ code in ``./cpp/``.
	* Expose your new features to Python by modifying ``./cpp/src/py_main.cpp``, ``./py/pybtls/lib/BTLS.py``, etc. accordingly.

	* For new features related to the data processing, modify the Python code in ``./py/pybtls/post_processing/``.

4. [If applicable] Create unit tests for ``pytest``.
    
    * Store test file in ``./tests/<test-file.py>``.

5. [If applicable] Create new example notebook.
    
    * Store notebook in ``./docs/source/notebooks/<tutorial.ipynb>``.
    * Index notebook in ``./docs/source/tutorial.rst``

6. [If applicable] Add new dependencies in ``./pyproject.toml``

7. Build documentation

	* Change directory to ``./docs/``
	* ``make clean``
	* ``make html``
	* ``xdg-open build/html/index.html``

8. Update version number in the following files

	* ``./pyproject.toml``
	* ``./py/pybtls/__init__.py``

9. Stage changes; commit; and push to remote fork

10. Go to GitHub and create PR for the branch
