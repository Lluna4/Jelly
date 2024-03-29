#pragma once
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
			index++;
		}

		void add_thread(std::function<void(std::stop_token, int)> func, int arg1)
		{
			threads.emplace_back(func, arg1);	
			index++;
		}
		template<typename T>
		void add_thread(std::function<void(std::stop_token, std::vector<T> &, int, bool *)> func, int arg1, std::vector<T> &arg2, bool *closed)
		{
			threads.emplace_back(func, std::ref(arg2), arg1, closed);
			index++;	
		}

		void stop_thread(int index)
		{
			if (index < threads.size())
				threads[index].~jthread();
			threads.erase(threads.begin() + index);
		}

		void request_stop_thread(int index)
		{
			if (index < threads.size())
				threads[index].request_stop();
			index--;
			threads.erase(threads.begin() + index);
		}

		void remove_from_list(int index)
		{
			threads.erase(threads.begin() + index);
			index--;
		}

		int get_current_index()
		{
			return index;
		}

		std::vector<int> get_flags()
		{
			return flags;
		}

		void flag(int index)
		{
			flags.push_back(index);
		}

		int flag_size()
		{
			return flags.size();
		}

	private:
		std::vector<std::jthread> threads;
		int index = -1;
		std::vector<int> flags;
};
