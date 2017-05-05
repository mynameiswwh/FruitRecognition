#include <iostream>
#include <opencv2/opencv.hpp>
#include <memory>
#include "segmentation/Quadtree.h"
#include "feature-extraction/Color.h"
#include "feature-extraction/Texture.h"
#include "feature-extraction/PrincipalComponentAnalysis.h"

using namespace cv;
using namespace std;

map<String, vector<String>> getAllFileNames();

String fruits[] = {
        "apples", "apricots", "avocados", "bananas", "blackberries", "blueberries",
        "cantaloupes", "cherries", "coconuts", "figs", "grapefruits", "grapes", "guava", "kiwifruit",
        "lemons", "limes", "mangos", "olives", "oranges", "passionfruit", "peaches", "pears", "pineapples",
        "plums", "pomegranates", "raspberries", "strawberries", "tomatoes", "watermelons"};


void segmentImage(Mat &rgbImage, const Mat &thresholdImage) {
    for (int row = 0; row < rgbImage.rows; ++row) {
        for (int col = 0; col < rgbImage.cols; ++col) {
            if (!thresholdImage.at<uchar>(row, col)) {
                Vec3b *pixel = rgbImage.ptr<Vec3b>(row, col);
                (*pixel)[0] = 255;
                (*pixel)[1] = 255;
                (*pixel)[2] = 255;
            }
        }
    }
}

map<String, vector<String>> getAllFileNames() {
    map<String, vector<String>> filenames;
    String folder = "FIDS30/";

    for (String fruit: fruits) {
        vector<String> filesForFruit;
        glob(folder + fruit, filesForFruit);
        filenames[fruit] = filesForFruit;
    }
    return filenames;
}

void fillHolesInThreshold(const Mat &grayImage, Mat &thresholdImage) {
    threshold(grayImage, thresholdImage, 0, 255,
              CV_THRESH_BINARY_INV + CV_THRESH_OTSU);

    // Floodfill from point (0, 0)
    Mat im_floodfill = thresholdImage.clone();
    floodFill(im_floodfill, Point(0, 0), Scalar(255));

    // Invert floodfilled image
    bitwise_not(im_floodfill, im_floodfill);

    // Combine the two images to get the foreground.
    thresholdImage = (thresholdImage | im_floodfill);
}

int main(int argc, char **argv) {
    namedWindow("Display frame", CV_WINDOW_AUTOSIZE);

//    vector<double> a = {2.5, 0.5, 2.2, 1.9, 3.1, 2.3, 2, 1, 1.5, 1.1};
//    vector<double> b = {2.4, 0.7, 2.9, 2.2, 3.0, 2.7, 1.6, 1.1, 1.6, 0.9};
//
//    shared_ptr<PrincipalComponentAnalysis> covTest(new PrincipalComponentAnalysis());
//    covTest->addFruitFeatures(a);
//    covTest->addFruitFeatures(b);
//
//    covTest->performPCA(14);

    map<String, vector<String>> filenames = getAllFileNames();

    for (int i = 0; i < fruits->length(); i++) {
        shared_ptr<PrincipalComponentAnalysis> pca(new PrincipalComponentAnalysis());

        for (String fruitFile: filenames[fruits[i]]) {
            Mat rgbImage;

            rgbImage = imread(fruitFile);

            if (!rgbImage.data) {
                cout << fruitFile << endl;
                printf("No image data \n");
            } else {
                Mat grayImage;
                cvtColor(rgbImage, grayImage, COLOR_RGB2GRAY);

                shared_ptr<Quadtree> root(new Quadtree(grayImage));
                root->splitAndMerge();

                Mat thresholdImage;
                fillHolesInThreshold(grayImage, thresholdImage);

                segmentImage(rgbImage, thresholdImage);
//                imshow("Display frame", rgbImage);

                vector<double> extractedFeatures = extractColorHistogram(rgbImage);

                pca->addFruitFeatures(extractedFeatures);
            }
        }
        pca->performPCA(0);
        waitKey(0);
    }
    return 0;
}

