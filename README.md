# C++实现的定时器

* 定时器设计基础来源于[baixiangcpp](https://gist.github.com/baixiangcpp/b2199f1f1c7108f22f47d2ca617f6960)。
* 在原作者基础上加入了`epoll`的控制，以及使用`pipe`处理优先队列为空时，超时时间为一个极大值的情况下，epoll进入较长等待的情况。
* 根据实际使用情况，精度设置为了1s，使用`time`获取，可根据原作者使用`clock_gettime`获取更加精准的时间。