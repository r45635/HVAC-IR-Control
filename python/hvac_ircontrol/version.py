# Version information.  
_version_major = 1
_version_minor = 1
_version_micro = 0


# An empty _version_extra corresponds to a full release.
# '.dev' as a _version_extra string means this is a development version
_version_extra = ''
#_version_extra = 'dev'


# Format expected by setup.py: string of form "X.Y.Z"
__version__ = "%s.%s.%s%s" % (_version_major,
                              _version_minor,
                              _version_micro,
                              _version_extra)
