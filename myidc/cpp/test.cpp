#include <iostream>  
#include <vector>  
#include <concepts>  
  
// 使用 C++20 的概念（concept）来定义一个模板函数  
template<typename T>  
concept Integral = std::is_integral_v<T>;  
  
// 使用 C++20 的范围（ranges）库中的算法（这里仅使用算法签名，不实际调用库）  
template<typename Range>  
requires std::ranges::range<Range>  
void print_range(Range&& range) {  
    // 这里只是一个示例，不实际使用范围库，仅输出一条消息  
    std::cout << "This function would iterate over the range if it used ranges library.\n";  
}  
  
// 注意：协程在 C++20 中是库特性，但它们的语法是语言级别的  
// 下面是一个协程的语法示例（但通常你需要一个支持协程的库来实际使用它）  
#include <coroutine> // 协程的头文件在 C++23 中可能是 <std/coroutine>，但在 C++20 中是库特性  
// ...  
// 协程函数示例（这里省略了实际实现）  
  
int main() {  
    // 使用 C++20 的概念来限制模板参数类型  
    std::vector<int> vec = {1, 2, 3, 4, 5};  
    print_range(vec); // 如果你的编译器支持范围库，可以取消注释这行代码  
  
    // 使用 C++20 的概念进行类型检查  
    if constexpr (Integral<int>) {  
        std::cout << "int is an integral type.\n";  
    }  
  
    // 这里可以添加更多 C++20 语法的示例，如 designated initializers, lambda capture by [=this] 等  
  
    return 0;  
}