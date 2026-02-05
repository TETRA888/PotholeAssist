#include <opencv2/opencv.hpp>
#include <cmath>

using namespace cv;
using namespace std;

/*
    Command to compile, link and run the code
    g++ gui.cpp -o gui $(pkg-config --cflags --libs opencv4) && ./gui
*/

int main() {
    VideoCapture cap(0);
    if (!cap.isOpened()) return -1;

    int width = (int)cap.get(CAP_PROP_FRAME_WIDTH);
    int height = (int)cap.get(CAP_PROP_FRAME_HEIGHT);

    Mat frame;
    double angle = 0.0;
    int radius = 63;
    int speed = 5;

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        Point center(width / 2, height * 3 / 4);

        // 1. Draw the filled Green body (thickness -1)
        circle(frame, center, radius, Scalar(0, 255, 125), -1);

        // --- NEW ---
        // 2. Draw the Black outline (thickness 3) at the same position
        circle(frame, center, radius, Scalar(0, 0, 0), 3);
        // -----------

        // 3. Draw rotating spokes
        for (int i = 0; i < 3; i++) {
            double theta = (angle + (i * 120)) * (CV_PI / 180.0);
            Point endPoint;
            endPoint.x = center.x + (radius * cos(theta));
            endPoint.y = center.y + (radius * sin(theta));
            line(frame, center, endPoint, Scalar(0, 0, 0), 3);
        }

        // 4. Draw center hub
        circle(frame, center, 10, Scalar(0, 0, 0), -1);

        /*
            Looking at drews cars perspective, draw lines along the lane,
            
            Then create logic that avoids said lanes by pivoting away from them
            
        */

        angle += speed;
        if (angle >= 360) angle = 0;

        imshow("Outlined Wheel", frame);
        if (waitKey(1) == 'q') break;
    }

    return 0;
}