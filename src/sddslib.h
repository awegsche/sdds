#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <variant>
#include <boost/regex.hpp>


namespace sdds {
	constexpr size_t BufSize = 1 << 20;

	const boost::regex ReDescriptor(R"(&(\w+) (.*) &end)");
	const std::string Str_Parameter("parameter");
	const std::string Str_Array("array");

	const boost::regex Re_Prop_Name(R"(name=(\w+),)");
	const boost::regex Re_Prop_Type(R"(type=(\w+),)");

	const boost::regex Re_Type_Float("float");
	const boost::regex Re_Type_Double("double");
	const boost::regex Re_Type_Long("long");
	const boost::regex Re_Type_String("string");

	namespace namelists {
		enum namelist_datatype {
			NDef,
			Float,
			Double,
			Long,
			String,
		};

		namelist_datatype datatype_from_str(const char* start, const char* end);

		struct description {};

		struct column {};

		struct parameter {
			namelist_datatype type;
			std::variant<float, double, long, std::string> value;

			parameter(namelist_datatype t) : type(t) {
				switch (type) {
				case Double:
					value = 0.0;
					break;
				case Float:
					value = 0.0f;
					break;
				case Long:
					value = 0L;
					break;
				case String:
					value = std::string{};
					break;
				}
			}

			void read(std::istream& stream);

			template<typename T>
			T& as() {
				return std::get<T>(value);
			}
		};

		struct sddsarray {
			namelist_datatype type;
			std::variant<
				std::vector<float>,
				std::vector<double>,
				std::vector<long>,
				std::vector<std::string>> payload;

			sddsarray(namelist_datatype t) : type(t) {
				switch (type) {
				case Double:
					payload = std::vector<double>{};
					break;
				case Float:
					payload = std::vector<float>{};
					break;
				case Long:
					payload = std::vector<long>{};
					break;
				case String:
					payload = std::vector<std::string>{};
				}
			}

			void read(std::istream& stream);

			template<typename T>
			T& as() {
				return std::get<T>(payload);
			}
		};

		struct namelist {
			enum name_list_type {
				NDef,
				Description,
				Column,
				Parameter,
				SddsArray,
			};

			static name_list_type type_from_str(const char* name);

			name_list_type type;
			std::string name;
			std::variant<description, parameter, sddsarray> payload;

			namelist() : type(NDef), name(), payload() {} 
			namelist(const namelist& other)
				: type(other.type), name(other.name), payload(other.payload)
			{ }

			template<typename T>
			T& as() {
				return std::get<T>(payload);
			}

			/// <summary>
			/// Loading the descriptor from the file header.
			/// Prepares the entry for data loading.
			/// </summary>
			/// <param name="buffer"></param>
			/// <param name="n"></param>
			void get_descriptor(char* buffer, std::streamsize n);

			/// <summary>
			/// Reading the value from the data page.
			/// Loads the actual data.
			/// </summary>
			/// <param name="stream"></param>
			void read_value(std::istream& stream);

			bool operator==(const std::string& str);
		};
	}

	class SddsFile {
	public:
		SddsFile(const std::string& filename);
		void load(const std::string& filename, size_t bufsize=BufSize);

		namelists::namelist& get_namelist(const std::string& name);

		template<typename T>
		T& get(const std::string& name) {
			auto& nl = get_namelist(name);
			return std::get<T>(nl.payload);
		}

	private:
		std::vector<namelists::namelist> data_;
	};

}

std::ostream& operator<<(std::ostream& os, const sdds::namelists::namelist& list);
std::ostream& operator<<(std::ostream& os, const sdds::namelists::parameter& param);
std::ostream& operator<<(std::ostream& os, const sdds::namelists::sddsarray& param);
