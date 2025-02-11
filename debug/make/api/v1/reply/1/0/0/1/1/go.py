import threading
import multiprocessing
import queue
import time
import os

class SingletonMeta(type):
    _instances = {}
    _lock = threading.Lock()  # Ensuring thread safety

    def __call__(cls, *args, **kwargs):
        with cls._lock:
            if cls not in cls._instances:
                instance = super().__call__(*args, **kwargs)
                cls._instances[cls] = instance
        return cls._instances[cls]

class Logger(metaclass=SingletonMeta):
    def __init__(self, filename="log.txt"):
        self.filename = filename
        self.log_queue = queue.Queue()
        self.lock = threading.Lock()
        self.running = True
        self.flush_thread = threading.Thread(target=self._flush_logs, daemon=True)
        self.flush_thread.start()

    def log(self, message):
        """Enqueues a log message for writing"""
        timestamp = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime())
        self.log_queue.put(f"[{timestamp}] {message}")
    
    def _flush_logs(self):
        """Background thread to flush logs to the file periodically"""
        while self.running or not self.log_queue.empty():
            try:
                with self.lock, open(self.filename, "a") as log_file:
                    while not self.log_queue.empty():
                        log_file.write(self.log_queue.get() + "\n")
            except Exception as e:
                print(f"Logging Error: {e}")
            time.sleep(1)  # Flush every second

    def close(self):
        """Ensures all logs are written before exiting"""
        self.running = False
        self.flush_thread.join()

def log_execution_time(func):
    def wrapper(*args, **kwargs):
        start_time = time.time()
        result = func(*args, **kwargs)
        elapsed = time.time() - start_time
        Logger().log(f"Function '{func.__name__}' executed in {elapsed:.4f}s")
        return result
    return wrapper

def fibonacci(n):
    if n <= 1:
        return n
    return fibonacci(n - 1) + fibonacci(n - 2)

@log_execution_time
def compute_fibonacci(n):
    """Runs Fibonacci computation in a separate process"""
    with multiprocessing.Pool(processes=4) as pool:
        result = pool.map(fibonacci, [n])
    Logger().log(f"Fibonacci({n}) = {result[0]}")
    return result[0]

def log_task(thread_id):
    for _ in range(5):
        Logger().log(f"Thread {thread_id}: Logging data...")
        time.sleep(0.1)

if __name__ == "__main__":
    try:
        logger = Logger()
        
        # Run logging in multiple threads
        threads = [threading.Thread(target=log_task, args=(i,)) for i in range(3)]
        for t in threads:
            t.start()
        for t in threads:
            t.join()
        
        # Compute Fibonacci in a separate process
        compute_fibonacci(10)

    except Exception as e:
        print(f"Error: {e}")

    finally:
        logger.close()
