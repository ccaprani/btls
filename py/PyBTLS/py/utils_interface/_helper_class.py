import pandas as pd
from abc import abstractmethod, abstractproperty

"""
Base class for expanded pandas dataframe
Allows importing data from a file, str object, or specified as dataframe by the user
Input:
    Select ONE of the following:
    1. Constructing from a file:
        'path': the path to the file to be imported
        **kwargs: Other kwargs as required, e.g. file_format for BtlsVehicle object
        After initialisation use import_from_file() method
    2. Constructing from a text:
        'text': The text to be converted into the object
        **kwargs: Other kwargs as required
        After initialisation use import_from_file() method
    3. Constructing as pandas DataFrame:
        Use pandas dataframe args and kwargs. Refer to pandas DataFrame documentation
        Not wholly supported after initialisation, though user can manipulate the existing DataFrame like regular
"""


class _DfBased(pd.DataFrame):
    def __init__(self, *args, **kwargs) -> None:
        self._check_args(*args, **kwargs)
        if "path" in kwargs:
            self._init_via_file(*args, **kwargs)
        elif "text" in kwargs:
            self._init_via_txt(*args, **kwargs)
        else:
            self._init_via_df(*args, **kwargs)

    def _check_args(self, *args, **kwargs):
        # Either supply path + file_format, text, or dataframe constructor args and kwargs
        num_mode_supplied = 0
        if "path" in kwargs:
            num_mode_supplied += 1
        if "text" in kwargs:
            num_mode_supplied += 1
        # Dataframe constructor must require data, index, columns, dtype, copy args or kwargs
        if (
            ("data" in kwargs)
            or ("index" in kwargs)
            or ("columns" in kwargs)
            or ("dtype" in kwargs)
            or ("copy" in kwargs)
            or len(args) > 0
        ):
            num_mode_supplied += 1
        if num_mode_supplied > 1:
            raise ValueError(
                "ERROR: Too many arguments supplied. Either supply only 'path', 'text' or pandas DataFrame args and kwargs. See note for help."
            )
        else:
            return 1

    # Separate the initialisation method so further classes can modify the behavious separately easily
    def _init_via_df(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def _init_via_file(self, *args, **kwargs):
        fpath = kwargs["path"]
        self.import_from_file(fpath, *args, **kwargs)

    def _init_via_txt(self, *args, **kwargs):
        text = kwargs["text"]
        self.import_from_txt(text, *args, **kwargs)

    def import_from_file(self, fpath, *args, **kwargs):
        with open(fpath) as f:
            text = f.read()
        super().__init__(**self._from_txt_to_dataframe_kwargs(text, *args, **kwargs))

    def import_from_txt(self, *args, **kwargs):
        super().__init__(**self._from_txt_to_dataframe_kwargs(*args, **kwargs))

    @abstractmethod
    def _from_txt_to_dataframe_kwargs(self, txt, *args, **kwargs):
        pass

    def __str__(self) -> str:
        qualname = type(self).__qualname__
        numrow = str(len(self.index))
        return f"{numrow} x {qualname} object at {hex(id(self))}"

    def __repr__(self) -> str:
        return super().__repr__()


r"""
ListLike base class. Provide a class where indexing is possible, just like a list
"""


class _ListLike:
    @abstractproperty
    def __primarykey__(self):
        pass

    @abstractproperty
    def __setitem_acceptable_type__(self):
        pass

    @property
    def __str_quantifier__(self):
        return "x"

    @property
    def __setitem_err_msg__(self):
        return (
            "ERROR: One or more values supplied is not of acceptable type: "
            + self.__setitem_acceptable_type__.__qualname__
        )

    def __delitem__(self, key):
        del eval("self." + self.__primarykey__)[key]

    def __getitem__(self, key):
        return eval("self." + self.__primarykey__)[key]

    def __setitem__(self, key, value):
        if not isinstance(value, self.__setitem_acceptable_type__):
            raise TypeError(self.__setitem_err_msg__)
        eval("self." + self.__primarykey__)[key] = value

    def __str__(self):
        qualname = type(self).__qualname__
        return f"{len(eval('self.' + self.__primarykey__))} {self.__str_quantifier__} {qualname} object at {hex(id(self))}"

    def __repr__(self):
        qualname = type(self).__qualname__
        name = [attr.__str__() for attr in eval("self." + self.__primarykey__)]
        name = name.__str__().replace(",", ",\n").replace("'", "")

        return f"{len(eval('self.' + self.__primarykey__))} {self.__str_quantifier__} {qualname} object at {hex(id(self))}\n{name}"
