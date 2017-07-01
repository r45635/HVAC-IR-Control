import os
from setuptools import setup, find_packages

# Get __version__ which is stored in python/version.py
ver_file = os.path.join('python', 'hvac_ircontrol', 'version.py')
exec(open(ver_file).read())

setup(
    name='hvac_ircontrol',
    package_dir = {'': 'python'},
    version=__version__,
    packages=find_packages(srcfile('python')),
    namespace_packages=['hvac_ircontrol'],
    url='',
    license='',
    author='ericmas001',
    author_email='ericmas001@gmail.com',
    description=''
)
