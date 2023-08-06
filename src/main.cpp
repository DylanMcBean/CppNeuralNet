#include <iostream>
#include <opencv2/opencv.hpp>
#include <thread>
#include <unordered_set>
#include <vector>

#include "Network.h"

std::vector<std::vector<double>> processRow(const cv::Mat& in_img, int y) {
  std::vector<std::vector<double>> row_data;
  row_data.reserve(in_img.cols - 2);

  for (int x = 1; x < in_img.cols - 1; x++) {
    std::vector<double> neighborhood;
    neighborhood.reserve(27);

    const uchar* previous_row = in_img.ptr<uchar>(y - 1);
    const uchar* current_row = in_img.ptr<uchar>(y);
    const uchar* next_row = in_img.ptr<uchar>(y + 1);

    for (int offset = -3; offset <= 3; offset += 3) {
      for (int ch = 0; ch < 3; ch++) {
        neighborhood.push_back(previous_row[3 * x + offset + ch] / 255.0);
        neighborhood.push_back(current_row[3 * x + offset + ch] / 255.0);
        neighborhood.push_back(next_row[3 * x + offset + ch] / 255.0);
      }
    }

    row_data.push_back(std::move(neighborhood));
  }

  return row_data;
}

std::vector<cv::Vec3b> processRowPixels(
    Network& network, const std::vector<std::vector<double>>& row_data) {
  Matrix input(row_data);

  network.SetInputs(&input);
  network.Forward();

  Matrix* output = network.outputs;

  int numRows = output->numRows();
  std::vector<cv::Vec3b> out_pixel_row(numRows);

  for (int i = 0; i < numRows; i++) {
    out_pixel_row[i] =
        cv::Vec3b((*output)(i, 0) * 255.0, (*output)(i, 1) * 255.0,
                  (*output)(i, 2) * 255.0);
  }

  return out_pixel_row;
}

void processImage(Network& network, const std::string& input_filename,
                  const std::string& output_filename) {
  cv::Mat in_img = cv::imread(input_filename);
  if (in_img.empty()) {
    std::cerr << "Failed to load image: " << input_filename << std::endl;
    return;
  }

  cv::Mat out_img = cv::Mat(in_img.rows, in_img.cols, in_img.type());

  for (int y = 1; y < in_img.rows - 1; y++) {
    std::vector<std::vector<double>> row_data = processRow(in_img, y);
    std::vector<cv::Vec3b> out_pixel_row = processRowPixels(network, row_data);

    for (int x = 1; x < in_img.cols - 1; x++) {
      out_img.at<cv::Vec3b>(y, x) = out_pixel_row[x - 1];
    }
  }

  cv::imwrite(output_filename, out_img);
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0]
              << " <input_filename> <output_filename> <network_file>"
              << std::endl;
    return 1;  // exit with an error code
  }

  std::string input_filename = argv[1];
  std::string output_filename = argv[2];
  std::string network_filename;

  if (argc == 4) {
    network_filename = argv[3];
  }

  Matrix X{{0}};

  // Create a Network object
  Network network(&X, 1);

  // Create LayerDense objects
  LayerDense layer1(27, 9, ActivationMethod::Sigmoid);
  LayerDense layer2(9, 6, ActivationMethod::ReLU);
  LayerDense layer3(6, 3, ActivationMethod::Softmax);

  // Add layers to the network
  network.AddLayer(&layer1);
  network.AddLayer(&layer2);
  network.AddLayer(&layer3);

  // load network from file (if it exists)
  if (!network_filename.empty()) {
    std::ifstream file_in(network_filename, std::ios::binary);
    if (file_in.good()) {
      std::cout << "Loading network from file: " << network_filename
                << std::endl;
      network.Load(file_in);
    }
  } else {
    std::cout << "No network file provided. Creating a new network."
              << std::endl;
  }

  processImage(network, input_filename, output_filename);

  // Save the network
  std::ofstream file_out("../network.bin", std::ios::binary);
  network.Save(file_out);
  return 0;
}
