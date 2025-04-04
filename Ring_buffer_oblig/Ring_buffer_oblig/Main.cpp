#include <string>
#include <thread>
#include <iostream>
#include <chrono>
#include "ringbuffer.h"

// Read from the buffer, and output to the console
void reader_thread(std::stop_token stop_token, ring_buffer<char>& buffer, int delay)
{
    while (!stop_token.stop_requested())
    {
        std::cout << buffer.pop();
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }
}

// Write from the keyboard to the buffer, until the user writes exit
void keyboard_thread(std::stop_token stop_token, ring_buffer<char>& buffer, int delay)
{
    std::string input;
    while (!stop_token.stop_requested())
    {
        std::getline(std::cin, input);
        if (input == "exit")
        {
            return;
        }

        for (char c : input)
        {
            buffer.push(c);
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        }
    }
}

int main()
{

	ring_buffer<char> buffer(10);

	std::jthread reader(reader_thread, std::ref(buffer), 100);
	std::jthread keyboard(keyboard_thread, std::ref(buffer), 100);

	// Wait for the threads to finish
	reader.join();
	keyboard.join();

	return 0;
}