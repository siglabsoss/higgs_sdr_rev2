###############################################################################
###############################################################################
# Name: log.py
# Coder: Janson Fang
# Description:
#   This module contains a method used create a logger
###############################################################################

###############################################################################
# Libraries and Modules
###############################################################################
import logging
import sys
###############################################################################
# Method Definitions
###############################################################################
def createLogger(logLevel = 'DEBUG'):
    '''Returns a configured logger named 'root'
    Log messages are in the format:
        '%(asctime)s [%(levelname)s] (%(module)s:%(lineno)d) %(message)s'
    Args:
        logLevel (str): A string indicating returned log level
    Returns
        A logger with where stream is set to sys.stdout and log level is user
        defined
    '''
    logger = logging.getLogger('root')
    logFormat = \
        '%(asctime)s [%(levelname)s] (%(module)s:%(lineno)d) %(message)s'

    formatter = logging.Formatter(logFormat, '%Y-%m-%d %H:%M:%S')

    handler = logging.StreamHandler(stream=sys.stdout)
    handler.setFormatter(formatter)
    logger.addHandler(handler)

    level = logging.getLevelName(logLevel)
    logger.setLevel(level)

    return logger
###############################################################################
# Main Script
###############################################################################
if __name__ == "__main__":
    logger = createLogger()
    logger.info('Test log statement: %d', 5)
else:
    logger = createLogger()