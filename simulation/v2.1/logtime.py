from functools import wraps
import time, os, psutil, logging

FILE_PATH = '/home/live/Desktop/ResilioSyn/pythonX/aPBR/v1/'


def my_logger(orig_func):
    format_ = '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    logging.basicConfig(filename= (FILE_PATH+'{}.log').format(orig_func.__name__),\
    format=format_, level=logging.DEBUG)
    # logging.disable(logging.CRITICAL)
    @wraps(orig_func)
    def wrapper(*args, **kwargs):
        logging.debug('Ran with args: {}, and kwargs: {}'.format(args, kwargs))
        return orig_func(*args, **kwargs)
    return wrapper

def my_timer(orig_func):
    @wraps(orig_func)
    def wrapper(*args, **kwargs):
        t1 = time.time()
        result = orig_func(*args, **kwargs)
        t2 = time.time() - t1
        print('{} ran in: {:.2f} sec'.format(orig_func.__name__, t2))
        return result
    return wrapper

def memory_usage_psutil(orig_func):
    # return the memory usage in MB
    @wraps(orig_func)
    def wrapper(*args, **kwargs):
        process = psutil.Process(os.getpid())
        mem = process.memory_info()[0] / float(2 ** 20)
        print('Memory used {:.1f} MB with {}'.format(mem, orig_func.__name__))
        return orig_func(*args, **kwargs)
    return wrapper

def datestamp():
    return time.strftime("%Y%m%d", time.localtime())
