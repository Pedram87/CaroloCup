#include "LineDetector.h"
#include <stdio.h>
#include <math.h>

#define USE_PPHT
#define MAX_NUM_LINES    100

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

namespace carolocup
{

using namespace std;
using namespace cv;

LineDetector::LineDetector(const Mat& f, const bool debug)
    : m_lines(NULL)
    , m_debug(debug)
    , m_lastSolidRightTop()
    , detectedLines()
    , supposedMidLine()
{
    m_frame = f.clone();
    Mat outputImg = f.clone();
    //if (m_debug)
    //imshow("m_frame",m_frame);
    // cvtColor( m_frame, m_frame, CV_BGR2GRAY );
    threshold( m_frame, m_frame, 175, 200, 0 );

    //cout<<"Thresholding Done............"<<endl;
    // Canny
    cv::Canny(m_frame, m_frameCanny, 180, 120, 3);
    //cout<<"Canny Done............"<<endl;
    // Create and init MSAC
    MSAC msac;
    int mode = MODE_NIETO;
    double w = m_frame.size().width;
    double h = m_frame.size().height;
    Size procSize = cv::Size(w, h);
    msac.init(mode, procSize, false);

    //cout<<"Initializing MSAC............"<<endl;
    // ++++++++++++++++++++++++++++++++++++++++
    // Process
    // ++++++++++++++++++++++++++++++++++++++++
    processImageMSAC(msac, 1, m_frame, outputImg);

    // View
    //if(m_debug) imshow("Before output", frame);
    //if(m_debug) imshow("Output", outputImg);
    //imshow("Output", outputImg);
}

LineDetector::~LineDetector()
{
    if (NULL != m_lines)
    {
        delete m_lines;
    }
}

Lines LineDetector::getLines()
{
    if (NULL == m_lines)
    {
        if(detectedLines.size() == 3)
        {
            cout<<"Lines == 3" << endl;
            m_lines = new Lines(Vec4i(detectedLines[0].p1.x, detectedLines[0].p1.y, detectedLines[0].p2.x, detectedLines[0].p2.y)
                                , Vec4i(detectedLines[1].p1.x, detectedLines[1].p1.y, detectedLines[1].p2.x, detectedLines[1].p2.y)
                                , Vec4i(detectedLines[2].p1.x, detectedLines[2].p1.y, detectedLines[2].p2.x, detectedLines[2].p2.y));
        }
        else if (detectedLines.size() == 2)
        {
            cout<<"Lines == 2" << endl;
            m_lines = new Lines(Vec4i(detectedLines[0].p1.x, detectedLines[0].p1.y, detectedLines[0].p2.x, detectedLines[0].p2.y)
                                , Vec4i(0,0,0,0)
                                , Vec4i(detectedLines[1].p1.x, detectedLines[1].p1.y, detectedLines[1].p2.x, detectedLines[1].p2.y));
        }
        else if (detectedLines.size() == 1)
        {
            cout<<"Lines == 1" << endl;
            if(supposedMidLine.slope > detectedLines[0].slope)
            {
                m_lines = new Lines(Vec4i(0,0,0,0)
                                    , Vec4i(0,0,0,0)
                                    , Vec4i(detectedLines[0].p1.x, detectedLines[0].p1.y, detectedLines[0].p2.x, detectedLines[0].p2.y));
            }
            else
            {
                m_lines = new Lines(Vec4i(detectedLines[0].p1.x, detectedLines[0].p1.y, detectedLines[0].p2.x, detectedLines[0].p2.y)
                                    , Vec4i(0,0,0,0)
                                    , Vec4i(0,0,0,0));
            }
        }
        else if(detectedLines.size() > 3)
        {
            cout<<"Lines > 3" << endl;
            int size = detectedLines.size();
            m_lines = new Lines(Vec4i(detectedLines[size/2-1].p1.x, detectedLines[size/2-1].p1.y, detectedLines[size/2-1].p2.x, detectedLines[size/2-1].p2.y)
                                , Vec4i(detectedLines[size/2].p1.x, detectedLines[size/2].p1.y, detectedLines[size/2].p2.x, detectedLines[size/2].p2.y)
                                , Vec4i(detectedLines[size/2+1].p1.x, detectedLines[size/2+1].p1.y, detectedLines[size/2+1].p2.x, detectedLines[size/2+1].p2.y));
        } 
        else 
        {
	    m_lines = new Lines(Vec4i(0,0,0,0)
                             , Vec4i(0,0,0,0)
                             , Vec4i(0,0,0,0));
        }
        m_lines.setSupposedMidLine(supposedMidLine);
    }
    return *m_lines;
}

/** This function contains the actions performed for each image*/
void LineDetector::processImageMSAC(MSAC &msac, int numVps, cv::Mat &imgGRAY, cv::Mat &outputImg)
{
    //cv::Mat imgCanny;

    // Canny
    //cv::Canny(imgGRAY, imgCanny, 180, 120, 3);

    // Hough
    vector<vector<cv::Point> > lineSegments;
    vector<cv::Point> aux;
#ifndef USE_PPHT
    vector<Vec2f> lines;
    cv::HoughLines( m_frameCanny, lines, 1, CV_PI/180, 200);



    for(size_t i=0; i< lines.size(); i++)
    {
        float rho = lines[i][0];
        float theta = lines[i][1];

        double a = cos(theta), b = sin(theta);
        double x0 = a*rho, y0 = b*rho;

        Point pt1, pt2;
        pt1.x = cvRound(x0 + 1000*(-b));
        pt1.y = cvRound(y0 + 1000*(a));
        pt2.x = cvRound(x0 - 1000*(-b));
        pt2.y = cvRound(y0 - 1000*(a));

        aux.clear();
        aux.push_back(pt1);
        aux.push_back(pt2);
        lineSegments.push_back(aux);

        line(outputImg, pt1, pt2, CV_RGB(0, 0, 0), 1, 8);

    }
#else


    //cout<<"Initializing HoughLines............"<<endl;
    vector<Vec4i> lines;
    int houghThreshold = 20;
    double w = imgGRAY.size().width;
    double h = imgGRAY.size().height;
    //if(imgGRAY.cols*imgGRAY.rows < 400*400)
    //houghThreshold = 100;
    //cout<<"Graying............"<<endl;

    //cout<<"Getting Channels:  "<< m_frameCanny.channels()<<endl;
    cv::HoughLinesP(m_frameCanny, lines, 1, CV_PI/180, houghThreshold, 10,10);
    //cout<<"Houghing!!!............"<<endl;
    while(lines.size() > MAX_NUM_LINES)
    {
        lines.clear();
        houghThreshold += 10;
        cv::HoughLinesP(m_frameCanny, lines, 1, CV_PI/180, houghThreshold, 10, 10);
    }
    cout << "Hough: " << houghThreshold << endl;
    for(size_t i=0; i<lines.size(); i++)
    {
        Point pt1, pt2;
        pt1.x = lines[i][0];
        pt1.y = lines[i][1];
        pt2.x = lines[i][2];
        pt2.y = lines[i][3];

        //cout<<"Scalar Stuffs............"<<endl;
        line(outputImg, pt1, pt2, Scalar(0), 2);

        //cout << "Hurray!!!: "<< endl;
        /*circle(outputImg, pt1, 2, CV_RGB(255,255,255), CV_FILLED);
        circle(outputImg, pt1, 3, CV_RGB(0,0,0),1);
        circle(outputImg, pt2, 2, CV_RGB(255,255,255), CV_FILLED);
        circle(outputImg, pt2, 3, CV_RGB(0,0,0),1);*/
        float slope = getLineSlope(pt1, pt2);

        // Store into vector of pairs of Points for msac
        // only if angle constraints are satisfied
        if(slope > 20 && slope < 160)
        {
            aux.clear();
            aux.push_back(pt1);
            aux.push_back(pt2);
            lineSegments.push_back(aux);
        }
    }
    //cout<<" HoughLines DONE!!!!!!!............"<<endl;
#endif

    // Multiple vanishing points
    std::vector<cv::Mat> vps;            // vector of vps: vps[vpNum], with vpNum=0...numDetectedVps
    std::vector<std::vector<int> > CS;    // index of Consensus Set for all vps: CS[vpNum] is a vector containing indexes of lineSegments belonging to Consensus Set of vp numVp
    std::vector<int> numInliers;

    std::vector<std::vector<std::vector<cv::Point> > > lineSegmentsClusters;

    // Call msac function for multiple vanishing point estimation
    msac.multipleVPEstimation(lineSegments, lineSegmentsClusters, numInliers, vps, numVps);
    for(unsigned int v=0; v<vps.size(); v++)
    {
        printf("VP %d (%.3f, %.3f, %.3f)", v, vps[v].at<float>(0,0), vps[v].at<float>(1,0), vps[v].at<float>(2,0));
        fflush(stdout);
        double vpNorm = cv::norm(vps[v]);
        if(fabs(vpNorm - 1) < 0.001)
        {
            printf("(INFINITE)");
            fflush(stdout);
        }
        printf("\n");
    }
    // Draw line segments according to their cluster
//    msac.drawCS(outputImg, lineSegmentsClusters, vps);
    // Paint line segments
    int vpFound = vps.size() > 0;
    std::vector<CustomLine> customLineSegments;
    CustomLine midLine;
    if(lineSegmentsClusters.size() > 0 && vpFound)
    {
        //cout << "number of lines: " << lineSegmentsClusters[0].size() << endl;
        Point midLow;
        midLow.x = w/2;
        midLow.y = h;
        for (unsigned int v = 0; v < vps.size(); v++)
        {
            Point vp;
            vp.x = vps[v].at<float>(0,0);
            vp.y = vps[v].at<float>(1,0);
            cout << "VP: (" << vp.x << "," << vp.y << ")" << endl;
            midLine.p1 = midLow;
            midLine.p2 = vp;
            midLine.slope = getLineSlope(midLow, vp);
            for(unsigned int i=0; i<lineSegmentsClusters[0].size(); i++)
            {
                CustomLine l1, l2;
                Point pt1 = lineSegmentsClusters[0][i][0];
                Point pt2 = lineSegmentsClusters[0][i][1];
                l1.p1 = pt1;
                l1.p2 = vp;
                l1.slope = getLineSlope(vp, pt1);
                customLineSegments.push_back(l1);
                l2.p1 = pt2;
                l2.p2 = vp;
                l2.slope = getLineSlope(vp, pt2);
                customLineSegments.push_back(l2);
                //cout << "Slope pt2: " << l2.slope << endl;
                //cout << "Slope pt1: " << l1.slope << endl;
                //cout << "Slope mid: " << midLine.slope << endl;
                //line(outputImg, vp, pt2, cv::Scalar(0,0,255), 2);
                //line(outputImg, vp, pt1, cv::Scalar(0,0,255), 2);
                //line(outputImg, vp, midLow, cv::Scalar(0,0,255), 2);
                //cout << "Line coord: [(" << pt1.x << "," << pt1.y << "),(" << pt2.x << "," << pt2.y << ")]" << endl;
                line(outputImg, pt1, pt2, cv::Scalar(0,255,0), 2);
            }
        }
        std::sort(customLineSegments.begin(), customLineSegments.end());
        //cout << "Lines in " << customLineSegments.size() << endl;
        CustomLine core = customLineSegments[0];
        //cout << "Slope mid: " << midLine.slope << endl;
        supposedMidLine = midLine;
        int countSameLines = 0;
        int countAllMergedLines = 0;
        float sameLinesLength = 0;
        float minLength = 2000;
        float maxLength = 0;
        //cout << "Prepare to draw!" << endl;
        for(unsigned int i=0; i<customLineSegments.size() - 1; i++)
        {
            //cout << "curr slope:" << customLineSegments[i].slope << endl;
            //cout << "Line coord: [(" << customLineSegments[i].p1.x << "," << customLineSegments[i].p1.y << "),(" << customLineSegments[i].p2.x << "," << customLineSegments[i].p2.y << ")]" << endl;
            if(abs(customLineSegments[i].slope - customLineSegments[i+1].slope) > 10 )
            {
                //cout << "DRAW" << endl;
                core.p1.y = h;
                float radianSlope = tan((core.slope/180.) * M_PI);
                float b = core.p2.y - radianSlope * core.p2.x;
                core.p1.x = (h - b) / radianSlope;
                //cout << "Same line :" << countSameLines << " " << countAllMergedLines << endl;
                //cout << "Same line len: " << sameLinesLength << endl;
                Scalar lineColor = cv::Scalar(255, 0, 0);
                if((countSameLines/(float)countAllMergedLines) > 0.6 && countAllMergedLines > 2 && sameLinesLength < 1000)
                {
                    lineColor = cv::Scalar(255, 255, 0);
                }
                detectedLines.push_back(core);
                line(outputImg, core.p1, core.p2, lineColor, 2);
                //cout << "Slope core: " << core.slope << endl;
                core = customLineSegments[i+1];
                countSameLines = 0;
                countAllMergedLines = 0;
                sameLinesLength = 0;
            }
            else
            {
                //cout << "MERGE" << endl;
                if(customLineSegments[i].slope < midLine.slope)
                {
                    core = customLineSegments[i];
                    //cout << "Line coord: [(" << core.p1.x << "," << core.p1.y << "),(" << core.p2.x << "," << core.p2.y << ")]" << endl;
                }
                float segmentLength = getDist(customLineSegments[i].p1, customLineSegments[i+1].p1);
                //cout << "Segment Length: " << segmentLength << endl;
                if ((fEqual(sameLinesLength,0)) && (segmentLength > 10))
                {
                    sameLinesLength = segmentLength;
                    minLength = segmentLength;
                    maxLength = segmentLength;
                    countSameLines = 1;
                }
                else
                {
                    float avg_len = (sameLinesLength*countSameLines + segmentLength) / (countSameLines + 1);
                    if(abs(avg_len - segmentLength) < 30 && segmentLength > 10)
                    {
                        sameLinesLength = avg_len;
                        countSameLines ++;
                        if(minLength > segmentLength)
                        {
                            minLength = segmentLength;
                        }
                        if(maxLength < segmentLength)
                        {
                            maxLength = segmentLength;
                        }
                    }
                    else
                    {
                        if((segmentLength > 10) && (((maxLength + 50) < segmentLength ) || (segmentLength < minLength - 50)))
                        {
                            sameLinesLength += 2000;
                            //countSameLines = 1;
                        }
                    }
                }
                countAllMergedLines++;
            }
        }
        //cout << "end drawing!" << endl;
        //cout << "Line coord: [(" << core.p1.x << "," << core.p1.y << "),(" << core.p2.x << "," << core.p2.y << ")]" << endl;
        Scalar lineColor = cv::Scalar(0,0,255);
        if((countSameLines/(float)countAllMergedLines) > 0.6 && countAllMergedLines > 2 && sameLinesLength < 1000)
        {
            lineColor = cv::Scalar(255, 255, 0);
        }
        //cout << "Same line :" << countSameLines << " " << countAllMergedLines << endl;
        //cout << "Same line len: " << sameLinesLength << endl;
        core.p1.y = h;
        float radianSlope = tan((core.slope/180.) * M_PI);
        float b = core.p2.y - radianSlope * core.p2.x;
        core.p1.x = (h - b) / radianSlope;
        detectedLines.push_back(core);
        line(outputImg, core.p1, core.p2, lineColor, 2);
        //cout << "Slope core: " << core.slope << endl;
    }
}

int LineDetector::detectHorizontalLine(Mat canny_roi, int dist)
{
    vector<Vec4i> lines;
    // Hough line detection
    HoughLinesP(canny_roi, lines, 1, CV_PI/180, 50, 100, 100);
    vector<Vec4i> likely_lines;
    for (vector<Vec4i>::iterator it = lines.begin(); it != lines.end(); it++)
    {
        int xA = (*it)[0], yA = (*it)[1];
        int xB = (*it)[2], yB = (*it)[3];
        double theta = atan2(yB-yA, xB-xA);
        //cout << "Angle: " << theta*180/CV_PI << endl;
        if (theta >= -CV_PI/36 && theta <= CV_PI/36)
        {
            //&&_roi.cols/2 && xB >= src_roi.cols/2) {
            likely_lines.push_back(*it);
            //cout << "(" << xA << ", " << yA << "), (" << xB << ", " << yB << ")" << endl;
        }
    }
    int yMax = 0;
    vector<Vec4i>::iterator ptr1, ptr2;
    for (vector<Vec4i>::iterator it1 = likely_lines.begin(); it1 != likely_lines.end(); it1++)
    {
        for (vector<Vec4i>::iterator it2 = it1+1; it2 != likely_lines.end(); it2++)
        {
            Point p1A = Point((*it1)[0], (*it1)[1]);
            Point p1B = Point((*it1)[2], (*it1)[3]);
            Point p2A = Point((*it2)[0], (*it2)[1]);
            Point p2B = Point((*it2)[2], (*it2)[3]);
            int y1Avg = (p1A.y+p1B.y)/2;
            int y2Avg = (p2A.y+p2B.y)/2;
            if (abs(y1Avg-y2Avg) <= dist)
            {
                if (max(y1Avg, y2Avg) > yMax)
                {
                    yMax = max(y1Avg, y2Avg);
                    ptr1 = it1;
                    ptr2 = it2;
                }
            }
        }
    }
    if (yMax > 0)
    {
        return canny_roi.rows-yMax;
    }
    else
    {
        return -1;
    }
}

int LineDetector::detectStartLine(int dist)
{
    Rect roi_left, roi_right;
    roi_left = Rect(0, m_frameCanny.rows/2, m_frameCanny.cols/2, m_frameCanny.rows/2);
    roi_right = Rect(m_frameCanny.cols/2, m_frameCanny.rows/2, m_frameCanny.cols/2, m_frameCanny.rows/2);
    int yLineLeft = detectHorizontalLine(m_frameCanny(roi_left), dist);
    int yLineRight = detectHorizontalLine(m_frameCanny(roi_right), dist);
    if (abs(yLineLeft-yLineRight) <= 10)
    {
        return min(yLineLeft, yLineRight);
    }
    else
    {
        return -1;
    }
}

int LineDetector::detectStopLine(int dist)
{
    Mat src_roi;
    // Cut out the lower right corner
    Rect roi = Rect(m_frameCanny.cols/2, m_frameCanny.rows/2, m_frameCanny.cols/2, m_frameCanny.rows/2);
    src_roi = m_frameCanny(roi);
    return detectHorizontalLine(src_roi, dist);
}

/** Get the slope of a line defined by two points */
float LineDetector::getLineSlope(Point &p1, Point &p2)
{
    float slope = M_PI/2;
    if((p1.x - p2.x)!= 0)
    {
        slope = (p1.y - p2.y) / ((float)(p1.x - p2.x));
        slope = atan(slope);
    }
    if(slope < 0)
    {
        return 180 + (slope/M_PI)*180;
    }
    return (slope/M_PI)*180;
}

/** Get distance between two points */
float LineDetector::getDist(const Point p1, const Point p2) const
{
    return sqrt(pow(p1.x-p2.x, 2) + pow(p1.y-p2.y, 2));
}

}
