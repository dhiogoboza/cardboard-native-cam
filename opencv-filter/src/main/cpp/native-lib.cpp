#include <jni.h>
#include <android/log.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#define INPUT_MAX 100

#define CARTOON_THICK_MIN 3
#define CARTOON_THICK_MAX 51
#define CARTOON_THRESH_MIN 1
#define CARTOON_THRESH_MAX 10

#define CONVERT_RANGE(param, rangeMin, rangeMax)    (param*(rangeMax-rangeMin)/INPUT_MAX + rangeMin)

#define TAG "NativeLib"

using namespace std;
using namespace cv;

static double mScaleFactor = 1.0;
static int mStepNum = 4;
static float mStepProbArr[4] = { 0.1f, 0.2f, 0.2f, 0.5f };
static uchar mColorCartoonLevels[4] = {0, 30, 50, 100};
static uchar mGrayCartoonLevels[4] = {10, 50, 100, 255};
static uchar mStepValArr[4];

Mat mSrcGray, mDstGray, mEdges, mEdgesGray, mSrcScaled, mDstScaled, mSketchTexture;

/* Get Quantization step levels based on given step probabilities */
void getQuantizeSteps(Mat& src, int stepNum, float* stepProbArr, uchar* stepValArr) {
    Mat hist;
    int histSize = 256;

    float range[] = { 0, 256 } ;
    const float* histRange = { range };

    calcHist(&src, 1, 0, Mat(), hist, 1, &histSize, &histRange, true, false);

    float sumHist = 0.0f;
    for(int i=0; i<histSize; i++)
        sumHist += hist.at<float>(i,0);
    hist = hist/sumHist;

    float stepProb = 0.0f;
    int stepIndex = 0;
    for(int i=0; i<histSize; i++) {
        stepProb += hist.at<float>(i,0);
        if(stepProb >= stepProbArr[stepIndex]) {
            stepValArr[stepIndex++]=i;
            stepProb = 0.0f;
        }
    }
    for(int i=stepIndex; i<stepNum; i++)
        stepValArr[i] = 255;
}


/* Quantize the src image into dst, using given step boundaries and dst pixel values */
void quantize(Mat& src, Mat& dst, uchar* stepValArr, uchar* dstValArr)
{
    uchar buffer[256];
    int j=0;
    for(int i=0; i!=256; ++i) {
        if(i > stepValArr[j])
            j++;
        buffer[i] = dstValArr[j];
    }
    Mat table(1, 256, CV_8U, buffer, sizeof(buffer));
    LUT(src, table, dst);
}

/* Color-Cartoon Filter Imaplementation */
void applyColorCartoon(Mat& src, Mat& dst, int edgeThickness, int edgeThreshold)
{
    edgeThickness = CONVERT_RANGE(edgeThickness, CARTOON_THICK_MIN, CARTOON_THICK_MAX);
    edgeThreshold = CONVERT_RANGE(edgeThreshold, CARTOON_THRESH_MIN, CARTOON_THRESH_MAX);

    edgeThickness *= mScaleFactor;
    if(edgeThickness%2 == 0) edgeThickness++;

    if(edgeThickness < CARTOON_THICK_MIN)
        edgeThickness = CARTOON_THICK_MIN;

    resize(src, mSrcScaled, Size(), mScaleFactor, mScaleFactor, INTER_LINEAR);

    GaussianBlur(mSrcScaled, mSrcScaled, Size(5,5), 0);
    cvtColor(mSrcScaled, mSrcGray, COLOR_RGBA2GRAY);

    getQuantizeSteps(mSrcGray, mStepNum, mStepProbArr, mStepValArr);
    quantize(mSrcGray, mDstGray, mStepValArr, mColorCartoonLevels);
    cvtColor(mDstGray, mDstScaled, COLOR_GRAY2RGBA);
    mDstScaled = 0.7*mSrcScaled + 0.7*mDstScaled;

    adaptiveThreshold(mSrcGray, mEdgesGray, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, edgeThickness, edgeThreshold);
    cvtColor(mEdgesGray, mEdges, COLOR_GRAY2RGBA);
    mDstScaled = mDstScaled - ~mEdges;

    resize(mDstScaled, dst, src.size(), 0, 0, INTER_LINEAR);
}

extern "C" {
void JNICALL
Java_com_example_nativeopencvandroidtemplate_MainActivity_adaptiveThresholdFromJNI(JNIEnv *env,
                                                                                   jobject instance,
                                                                                   jlong matAddr) {

    // get Mat from raw address
    Mat &mat = *(Mat *) matAddr;

    clock_t begin = clock();

    //cv::adaptiveThreshold(mat, mat, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 21, 5);
    applyColorCartoon(mat, mat, 40, 50);

    // log computation time to Android Logcat
    double totalTime = double(clock() - begin) / CLOCKS_PER_SEC;
    __android_log_print(ANDROID_LOG_INFO, TAG, "adaptiveThreshold computation time = %f seconds\n",
                        totalTime);
}
}