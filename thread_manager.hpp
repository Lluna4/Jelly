# include <thread>
# include <iostream>
# include <vector>
# include <functional>

class thread_man
{
	public:
		thread_man() {}
		
		void stop_all_threads()
		{
			for (int i = 0; i < threads.size(); i++)
			{
				threads[i].~jthread();
				threads.erase(threads.begin() + i);
			}	
		}

		void add_thread(std::function<void(std::stop_token, std::string, int)> func, std::string arg1, int arg2)
		{
			threads.emplace_back(func, arg1, arg2);	
		}

		void add_thread(std::function<void(std::stop_token, int)> func, int arg1)
		{
			threads.emplace_back(func, arg1);	
		}
		template<typename T>
		void add_thread(std::function<void(std::stop_token, std::vector<T> &, int)> func, int arg1, std::vector<T> &arg2)
		{
			threads.emplace_back(func, std::ref(arg2), arg1);	
		}

		void stop_thread(int index)
		{
			if (index < threads.size())
				threads[index].~jthread();
			threads.erase(threads.begin() + index);
		}
	private:
		std::vector<std::jthread> threads;
};
