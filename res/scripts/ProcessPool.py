import multiprocessing


class ProcessPool:
    def __init__(self, processes):
        self._pool = multiprocessing.Pool(processes)

    def execute(self, function, args, kwargs):
        return self._pool.apply_async(function, args=args, kwds=kwargs).get()

    def __del__(self):
        self._pool.close()