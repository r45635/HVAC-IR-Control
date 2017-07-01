__import__('pkg_resources').declare_namespace('hvac_ircontrol')
from .version import __version__

from .ir_sender import *
from .mitsubishi import *
