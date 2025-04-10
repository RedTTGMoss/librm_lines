from poetry.core.masonry.api import build_wheel
import pathlib

def build_wheel_w_platform_tag(wheel_directory, config_settings=None, metadata_directory=None):
    dist_dir = pathlib.Path(wheel_directory)
    return build_wheel('.', dist_dir, config_settings={"--plat-name": "platform-specific"})

# PEP 517 requires this symbol:
__requires__ = ["poetry-core"]
