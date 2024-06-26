import pybtls
import os
import shutil


# GarageLoad
def garage_load(
    garage_path: str, garage_format: int
) -> list[pybtls.Library.BTLS_collections._Vehicle]:
    vehicle_list = pybtls.GarageProcessing.load_garage_file(garage_path, garage_format)
    pybtls.GarageProcessing.get_gvw_from_garage(vehicle_list)
    pybtls.GarageProcessing.get_vehicle_length_from_garage(vehicle_list)

    return vehicle_list


# GarageDivide
def garage_divide(
    vehicle_list: list[pybtls.Library.BTLS_collections._Vehicle],
) -> list[pybtls.GarageProcessing.divide.SubWIMDict]:
    sub_garage_list = []

    criterion = pybtls.GarageProcessing.region_criteria.AUCriterion()
    sub_garage_list.append(
        pybtls.GarageProcessing.divide_by_regional_classification(
            vehicle_list, criterion
        )
    )
    sub_garage_list.append(pybtls.GarageProcessing.divide_by_no_axle(vehicle_list))
    sub_garage_list.append(
        pybtls.GarageProcessing.divide_by_gvw_percentile(vehicle_list, [0.5, 1.0])
    )
    sub_garage_list.append(
        pybtls.GarageProcessing.divide_by_vehicle_length_percentile(
            vehicle_list, [0.5, 1.0]
        )
    )

    return sub_garage_list


# GarageWrite
def garage_write(
    sub_garage_list: list[pybtls.GarageProcessing.divide.SubWIMDict],
    output_folder_path: str,
    garage_format: int,
):
    for sub_garage in sub_garage_list:
        pybtls.GarageProcessing.write_divide_garage_file(
            sub_garage, output_folder_path, garage_format
        )


# GarageAnalyse
def garage_analyse(
    vehicle_list: list[pybtls.Library.BTLS_collections._Vehicle],
    output_folder_path: str,
):
    pybtls.GarageProcessing.do_gvw_stat(vehicle_list, False)
    pybtls.GarageProcessing.do_axle_weight_stat(vehicle_list, False)
    pybtls.GarageProcessing.do_vehicle_length_stat(vehicle_list, False)
    pybtls.GarageProcessing.do_axle_spacing_stat(
        vehicle_list, True, plot_save_path=output_folder_path + "/axle_spacing"
    )


# GarageAssemble
def garage_assemble(
    sub_garage_path: list[str], sub_garage_percentage: list[float], garage_format: int
):
    pybtls.GarageProcessing.assemble_garage(
        sub_garage_path, sub_garage_percentage, garage_format, garage_size=1000
    )


def test_garage_processing():
    current_dir = os.path.dirname(os.path.realpath(__file__))
    garage_path = "garage.txt"
    garage_path = os.path.join(current_dir, "test_data", garage_path)
    output_folder_path = "test_output"
    output_folder_path = os.path.join(current_dir, output_folder_path)
    garage_format = 4

    try:
        os.mkdir(output_folder_path)
    except:
        shutil.rmtree(output_folder_path)
        os.mkdir(output_folder_path)

    vehicle_list = garage_load(garage_path, garage_format)
    sub_garage_list = garage_divide(vehicle_list)
    garage_write(sub_garage_list, output_folder_path, garage_format)
    garage_analyse(vehicle_list, output_folder_path)
    garage_assemble([garage_path], [1.0], garage_format)

    shutil.rmtree(output_folder_path)
