//
// chat_message.hpp
// ~~~~~~~~~~~~~~~~
//
// Edited by Mateusz Wszolek
//

#ifndef CHAT_MESSAGE_HPP
#define CHAT_MESSAGE_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>

#include "definitions.h"

class chat_message
{
	public:
		enum { header_data = sizeof(uint32_t) * 4};
		enum { header_length = 4 + header_data};
		enum { max_body_length = 512 };
		enum { item_s = sizeof(uint32_t),
				item_s2 = item_s*2,
				item_s3 = item_s*3};

		chat_message()
			: body_length_(0)
		{
		}

		const char* data() const
		{
			return data_;
		}

		char* data()
		{
			return data_;
		}

		std::size_t length() const
		{
			return header_length + body_length_;
		}

		const char* body() const
		{
			return data_ + header_length;
		}

		char* body()
		{
			return data_ + header_length;
		}

		std::size_t body_length() const
		{
			return body_length_;
		}

		void body_length(std::size_t new_length)
		{
			body_length_ = new_length;
			if (body_length_ > max_body_length)
				body_length_ = max_body_length;
		}

		bool decode_header()
		{
			char header[header_length + 1] = "";
			char header_blength[5] = "";
			std::strncat(header, data_, header_length);
			std::strncat(header_blength, header, 4);
			body_length_ = std::atoi(header_blength);
			uint32_t *header_data = (uint32_t*) (data_+4); type_ = *header_data;
			header_data++; code_num_ = *header_data;
			header_data++; from_ = *header_data;
			header_data++; to_ = *header_data;
			if (body_length_ > max_body_length)
			{
				body_length_ = 0;
				return false;
			}
			return true;
		}

		void encode_header()
		{
			char header[header_length + 1] = "";
			std::sprintf(header, "%4d", body_length_);
			std::memcpy(data_, header, header_length);
			uint32_t *header_data = (uint32_t*) (data_+4); *header_data = type_;
			header_data++; *header_data = code_num_;
			header_data++; *header_data = from_;
			header_data++; *header_data = to_;
		}

		void set_data(uint32_t type, uint32_t code_num, uint32_t from,
				uint32_t to) {
			type_ = type;
			code_num_ = code_num;
			from_ = from;
			to_ = to;
		}

		void print_message(void) {
			std::cout << "type: " << type_ << "code_num: " << code_num_
				<< std::endl << "from: " << from_ << "to: " << to_ <<std::endl;
			std::cout << "length: " << body_length_;
			if (body_length_ > 0) {
				std::cout << "messgage: " << body() << std::endl;
				std::cout << "bytes: ";
				for (int i = 0; i < body_length_; ++i) {
					std::printf("%2X ", *(body() + i));
				}
			}
		}

		uint32_t get_type() { return type_; }
		uint32_t get_code_num() { return code_num_; }
		uint32_t get_from() { return from_; }
		uint32_t get_to() { return to_; }

	private:
		char data_[header_length + max_body_length];
		std::size_t body_length_;
		uint32_t type_;
		uint32_t code_num_;
		uint32_t from_;
		uint32_t to_;
};

#endif // CHAT_MESSAGE_HPP
