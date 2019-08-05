
try:
    import configparser
except ImportError:
    import ConfigParser as configparser

Import("env")
print(env.Dump())

config = configparser.ConfigParser()
config.read("platformio.ini")

progname_prefix = config.get("common","progname_prefix")
variant = env['PIOENV']
ver = config.get("common","ver")


env.Replace(PROGNAME="%s%s_%s" % (progname_prefix, variant, ver))