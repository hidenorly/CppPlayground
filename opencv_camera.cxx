/*
  Copyright (C) 2024 hidenorly

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <output_filename>" << std::endl;
        return 1;
    }

    std::string output_filename = argv[1];

    // open camera
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Error: Unable to open the camera." << std::endl;
        return 1;
    }

    std::cout << "Press 's' to take a photo and save it as " << output_filename << "." << std::endl;

    cv::Mat frame;
    while (true) {
        // grab frame
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "Error: Captured empty frame." << std::endl;
            break;
        }

        // show frame
        cv::imshow("Camera", frame);

        // key check
        char key = cv::waitKey(1);
        if (key == 's') {
            // s key to save the photo
            if (cv::imwrite(output_filename, frame)) {
                std::cout << "Photo saved as " << output_filename << "." << std::endl;
            } else {
                std::cerr << "Error: Failed to save the photo." << std::endl;
            }
            break;
        } else if (key == 27) { // Esc key to exit
            std::cout << "Exiting without saving." << std::endl;
            break;
        }
    }

    // release
    cap.release();
    cv::destroyAllWindows();

    return 0;
}
