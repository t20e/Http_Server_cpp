#include <bcrypt.h>
#include <exception>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <random>
#include <set>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/kazuho-picojson/defaults.h>


#include "request_handlers/ImageHandler.h"
#include "utils/Logger.h"
#include "utils/http_data.h"


#include "./request_handlers/ImageHandler.h"


ImageHandler::ImageHandler(
	Config &config,
	DB_controller &db,
	ResponseService &responseService)
	: BaseHandler(config, db, responseService) {}


int ImageHandler::getRandomImage(const HttpRequest &req, const int &client_socket)
{
	std::string images_directory_path = "./server_images";

	std::set<std::string> img_extensions = {
		".jpg", ".jpeg", ".png"};

	try {
		std::vector<std::filesystem::path> found_images;

		// Iterate over the directory and collect valid image paths
		for (const auto &entry: std::filesystem::directory_iterator(images_directory_path)) {
			if (entry.is_regular_file()) {
				std::string ext = entry.path().extension().string();
				if (img_extensions.count(ext)) { // Match to an image extension
					found_images.push_back(entry.path());
				}
			}
		}

		// Handle case where no images are found.
		if (found_images.empty()) {
			Logger::getInstance().log(LogLevel::CRITICAL, std::format("No images found in: {}, when attempting to get a random image!", images_directory_path));
			resService_.sendResponse(resService_.createResponse("No images found", 500, req.headers.at("Origin")), client_socket);
			return 1;
		}

		// Select a random image
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> distrib(0, found_images.size() - 1);

		std::filesystem::path selected_file = found_images[distrib(gen)];
        Logger::getInstance().log(LogLevel::DEBUG, std::format("Random Image Path: {}", selected_file.string()));

		// Open the image file in binary mode
		std::ifstream file(selected_file, std::ios::binary);

		if (!file.is_open()) {
			resService_.sendResponse(resService_.createResponse("Server Error: could not retrieve image", 500, req.headers.at("Origin")), client_socket);
			return 1;
			;
		}

		// Read the entire file into a string buffer
		std::string image_buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

		// Determine the image extension content-type to create a response object
		std::string image_ext = selected_file.extension().string();
		std::string content_type = "image/jpeg"; // default

		if (image_ext == ".png")
			content_type = "image/png";

		// Construct response
		std::string headers = std::format(
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: {}\r\n"
			"Content-Length: {}\r\n"
			// ----- Headers
			"Access-Control-Allow-Origin: {}\r\n" // Connection to frontend
			"Access-Control-Allow-Credentials: true\r\n" // allow cookies
			"Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
			"Access-Control-Allow-Headers: Content-Type\r\n" // allow json content
			// -----
			"Connection: close\r\n"
			"\r\n",
			content_type,
			image_buffer.size(),
			req.headers.at("Origin")

		);

		// Respond
		// Send the preflight
		send(client_socket, headers.c_str(), headers.size(), 0);
		// Send the POST image buffer data
		send(client_socket, image_buffer.data(), image_buffer.size(), 0);
		return 0;

	} catch (const std::filesystem::filesystem_error &err) {
		Logger::getInstance().log(LogLevel::ERROR, std::format("Unknown error: {}", err.what()));
		resService_.sendResponse(resService_.createResponse("Server error: getting a random image.", 500, req.headers.at("Origin")), client_socket);
	} catch (const std::exception &err) {
		Logger::getInstance().log(LogLevel::ERROR, std::format("Unknown error: {}", err.what()));
		resService_.sendResponse(resService_.createResponse("Unknown Server error: getting a random image.", 500, req.headers.at("Origin")), client_socket);
	}
	return 0;
}
