#pragma once
//这一个头文件是线程池的相关代码。
#include <mutex>
#include <future>
#include <condition_variable>
#include <queue>
#include <functional>
#include <atomic>

class TaskQueue
{
public:
    TaskQueue() {}
    TaskQueue(const TaskQueue&) = delete;
    template<typename F, typename ...Args> auto submit(F&& f, Args&& ...args) -> std::future<decltype(f(args...))>
    {
        using task_type = decltype(f(args...));
        std::function<task_type()> func_first_warp = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        //这一层处理的目的是把函数和传入值包装在一起变成一个整体，便于放入packaged_task当中
        //注意尖角括号中类型名要加括号，否则会报错不完整的类型
        auto task_ptr = std::make_shared<std::packaged_task<task_type()>>(func_first_warp);
        //这一层的处理是让一个智能指针指向第一次处理之后的函数，之后就可以直接通过智能指针来调用
        std::lock_guard<std::mutex> this_lock_guard(task_mutex);    //上锁，之后会有互斥数据
        task_queue.emplace([task_ptr]{(*task_ptr)();});
        return task_ptr->get_future();
    }
    //提交任务的函数，把任务压到队列中，传入参数是函数和它的传入参数
    std::function<void()> getTask()    //返回值是包装好的function类型
    {
        std::function<void()> task;
        std::lock_guard<std::mutex> task_locker(task_mutex);    //先上锁
        if(!empty())    //如果在任务池中还有任务
        {
            task = std::move(task_queue.front());   //从队列最前端取出任务
            task_queue.pop();   //删除原先在队列最前端的任务
            return task;
        }
        return nullptr; //如果队列中已经没有任务了，就返回空指针
    }
    bool empty() { return task_queue.empty(); }
    
private:
    std::mutex task_mutex;
    std::queue<std::function<void()>> task_queue;   //task_queue这个队列内部会存储function<void()>类型的元素
};

class ThreadPool
{
private:
    std::atomic<int> current_task_num;
    bool if_shutdown;
    TaskQueue task_queue;
    std::condition_variable not_empty_caller;
    std::mutex tp_mtx;
    std::vector<std::thread> task_vector;
public:
    ThreadPool() = delete;
    ThreadPool(int thread_num)
    {
        current_task_num = 0;
        if_shutdown = false;
        for(int i = 0; i < thread_num; i++)
        {
            task_vector.push_back(std::thread(&ThreadPool::worker, this));  //thread用成员函数构造的方式
            task_vector.back().detach();
            //压入一个线程之后，把这个线程detach掉，独立于主线程而运行。
            //线程当中运行的程序段是worker成员函数。
        }
    }
    ThreadPool(const ThreadPool&) = delete;
    ~ThreadPool()
    {
        if_shutdown = true;
        not_empty_caller.notify_all();
        //析构函数把if_shutdown变为true，同时唤醒所有线程，这样所有线程就都不会被阻塞，直接运行到线程结束，所有线程回收。
    }
    void worker()
    {
        while(!if_shutdown)
        //只要线程池没有被shutdown，循环一直会运作
        {
            std::unique_lock<std::mutex> thread_locker(tp_mtx); //先上锁
            not_empty_caller.wait(thread_locker, [this]{ return !task_queue.empty() || if_shutdown; });
            //如果队列不为空或者线程池没有被关闭就向下执行，否则等待一个任务push完成给这个线程发送消息
            if(!task_queue.empty())
            {
                auto current_task = std::move(task_queue.getTask());    //从任务队列中获取一个任务
                thread_locker.unlock(); //获取完之后就没有互斥量了，解锁
                current_task_num++; //当前正在运行的线程数加1，原子变量有非互斥性
                current_task(); //被包装好的function，直接这样去执行
                current_task_num--; //执行完之后正在运行的线程数减1
            }
        }
    }
    template<typename F, typename ...Args> auto push(F&& f, Args&& ...args) -> decltype(task_queue.submit(std::forward<F>(f), std::forward<Args>(args)...))
    {
        auto ret = task_queue.submit(std::forward<F>(f), std::forward<Args>(args)...);
        not_empty_caller.notify_one();
        return ret;
    }
    //push函数会把传入的函数和传入值转发给TaskQueue变量的submit函数
    int getTaskNum()
    {
        std::lock_guard<std::mutex> locker(tp_mtx);
        return current_task_num;
    }
};