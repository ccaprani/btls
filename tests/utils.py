import pycba as cba
from pathlib import Path

__all__ = ["remove_folder", "get_cba_IL"]


def remove_folder(folder):
    path = Path(folder)
    if path.exists() and path.is_dir():
        for child in path.iterdir():
            if child.is_file():
                child.unlink()
            else:
                remove_folder(child)
        path.rmdir()


def get_cba_IL(
    beam_string: str, beam_position: float, beam_LE: str, resolution: float = 0.1
):

    (L, EI, R, eType) = cba.parse_beam_string(beam_string)
    ils = cba.InfluenceLines(L, EI, R, eType)
    ils.create_ils(step=resolution)
    x, eta = ils.get_il(beam_position, beam_LE)

    return x, eta
