#include <opencv2\opencv.hpp>
#include <iostream>
#include <string>
#include <stdio.h>
//#include <math.h>
//#include "rect_detector.h"
//#include "artifacts_detector.h"
//#include "mura_detector.h"
//#include "curve_detector.h"
#include <cv.hpp>
using namespace cv;
using namespace std;


//����Ҷ�ͼ�����
IplImage *g_GrayImage = NULL;
//�����ֵ��ͼƬ����
IplImage *g_BinaryImage = NULL;
//�����ֵ�����ڱ���
const char *WindowBinaryTitle = "��ֵ��ͼƬ";
//���廬����Ӧ����

//����Դͼ�񴰿ڱ������
const char *WindowSrcTitle = "�Ҷ�ͼ��";
//��������������
const char *TheSliderTitle = "��ֵ����ֵ";




//const char *SrcPath = "C:\\Users\\lenovo\\Desktop\\1.jpg"; ////����ͼƬ·��

IplImage *g_pGrayImage_liantong = NULL;
IplImage *g_pBinralyImage_liantong = NULL;

int contour_num = 0; //���ֱ��
char  number_buf[10];  ////���ֱ�Ŵ������飬puttext

#define num_col 11   ////��ά������У�ÿһ����ȱ����Ϣ����ϸ��Ϣ

long int liantong_all_area = 0; ////��ͨ���������
long int Rect_all_area = 0;  //// ������С��Ӿ����ܵ����


////=====================================================================
struct my_struct1{
	double scale;  //// ������ʾͼ��ı���
	const int threshold_value_binaryzation;  ////�����һ�ζ�ֵ����ֵ
	const int threshold_value_second_binaryzation;  ////����ڶ��ζ�ֵ����ֵ
};
my_struct1 picture = { 0.5, 50, 100 };

////=====================================================================
struct my_struct2{
	int Model1_k1;  ////ͼ�����͸�ʴ
	int Model1_k2;  ////ͼ�����͸�ʴ
	int Model2_k1;  ////ͼ�����͸�ʴ
	int Model2_k2;  ////ͼ�����͸�ʴ
};
my_struct2 value = { 5, 2, 3, 2 };

////=====================================================================
struct my_struct3{

	double maxarea;  ////���ȱ�����
	double minarea;  ////��С��ʾ������ȱ�����

	double font_scale;  ////�����С
	int font_thickness; ////�����ϸ

	const int Feature_value2_number; ////����һ����ά������У���ȱ�ݵĸ���

};
my_struct3 value2 = { 5000, 1000, 0.6, 0.8, 100 };

////=====================================================================
struct my_struct4{

	const int hough_Canny_thresh1;
	const int hough_Canny_thresh2;
	const int hough_Canny_kernel;

	const int cvHoughLines2_thresh; ////����ֵ���ڶ��ٲ���ʾ��ֵԽ����ʾ���߶�Խ��
	const int cvHoughLines2_param1; ////��ʾ�߶ε���С����
	const int cvHoughLines2_param2; ////�߶�֮��� ��С���

};
my_struct4 Hough = { 50, 100, 3, 50, 20, 10 };


void fft2(IplImage *src, IplImage *dst)
{   //ʵ�����鲿  
	IplImage *image_Re = 0, *image_Im = 0, *Fourier = 0;
	//   int i, j;  
	image_Re = cvCreateImage(cvGetSize(src), IPL_DEPTH_64F, 1);  //ʵ��  
	//Imaginary part  
	image_Im = cvCreateImage(cvGetSize(src), IPL_DEPTH_64F, 1);  //�鲿  
	//2 channels (image_Re, image_Im)  
	Fourier = cvCreateImage(cvGetSize(src), IPL_DEPTH_64F, 2);
	// Real part conversion from u8 to 64f (double)  
	cvConvertScale(src, image_Re);
	// Imaginary part (zeros)  
	cvZero(image_Im);
	// Join real and imaginary parts and stock them in Fourier image  
	cvMerge(image_Re, image_Im, 0, 0, Fourier);

	// Application of the forward Fourier transform  
	cvDFT(Fourier, dst, CV_DXT_FORWARD);
	cvReleaseImage(&image_Re);
	cvReleaseImage(&image_Im);
	cvReleaseImage(&Fourier);
}

void fft2shift(IplImage *src, IplImage *dst)
{
	IplImage *image_Re = 0, *image_Im = 0;
	int nRow, nCol, i, j, cy, cx;
	double scale, shift, tmp13, tmp24;
	image_Re = cvCreateImage(cvGetSize(src), IPL_DEPTH_64F, 1);
	//Imaginary part  
	image_Im = cvCreateImage(cvGetSize(src), IPL_DEPTH_64F, 1);
	cvSplit(src, image_Re, image_Im, 0, 0);
	//����ԭ���������˹����ͼ����p123  
	// Compute the magnitude of the spectrum Mag = sqrt(Re^2 + Im^2)  
	//���㸵��Ҷ��  
	cvPow(image_Re, image_Re, 2.0);
	cvPow(image_Im, image_Im, 2.0);
	cvAdd(image_Re, image_Im, image_Re);
	cvPow(image_Re, image_Re, 0.5);
	//�����任����ǿ�Ҷȼ�ϸ��(���ֱ任ʹ��խ���ͻҶ�����ͼ��ֵӳ��  
	//һ������ֵ������ɼ�������˹����ͼ����p62)  
	// Compute log(1 + Mag);  
	cvAddS(image_Re, cvScalar(1.0), image_Re); // 1 + Mag  
	cvLog(image_Re, image_Re); // log(1 + Mag)  

	//Rearrange the quadrants of Fourier image so that the origin is at the image center  
	nRow = src->height;
	nCol = src->width;
	cy = nRow / 2; // image center  
	cx = nCol / 2;
	//CV_IMAGE_ELEMΪOpenCV����ĺ꣬������ȡͼ�������ֵ����һ���־��ǽ������ı任  
	for (j = 0; j < cy; j++){
		for (i = 0; i < cx; i++){
			//���Ļ���������ݳ��Ŀ���жԽǽ���  
			tmp13 = CV_IMAGE_ELEM(image_Re, double, j, i);
			CV_IMAGE_ELEM(image_Re, double, j, i) = CV_IMAGE_ELEM(
				image_Re, double, j + cy, i + cx);
			CV_IMAGE_ELEM(image_Re, double, j + cy, i + cx) = tmp13;

			tmp24 = CV_IMAGE_ELEM(image_Re, double, j, i + cx);
			CV_IMAGE_ELEM(image_Re, double, j, i + cx) =
				CV_IMAGE_ELEM(image_Re, double, j + cy, i);
			CV_IMAGE_ELEM(image_Re, double, j + cy, i) = tmp24;
		}
	}
	//��һ�����������Ԫ��ֵ��һΪ[0,255]  
	//[(f(x,y)-minVal)/(maxVal-minVal)]*255  
	double minVal = 0, maxVal = 0;
	// Localize minimum and maximum values  
	cvMinMaxLoc(image_Re, &minVal, &maxVal);
	// Normalize image (0 - 255) to be observed as an u8 image  
	scale = 255 / (maxVal - minVal);
	shift = -minVal * scale;
	cvConvertScale(image_Re, dst, scale, shift);
	cvReleaseImage(&image_Re);
	cvReleaseImage(&image_Im);
}


////=====================================================================
//����Ӧ��ֵ�˲�
uchar adaptiveProcess(const Mat &im, int row, int col, int kernelSize, int maxSize)
{
	vector<uchar> pixels;
	for (int a = -kernelSize / 2; a <= kernelSize / 2; a++)
		for (int b = -kernelSize / 2; b <= kernelSize / 2; b++)
		{
			pixels.push_back(im.at<uchar>(row + a, col + b));
		}
	sort(pixels.begin(), pixels.end());
	auto min = pixels[0];
	auto max = pixels[kernelSize * kernelSize - 1];
	auto med = pixels[kernelSize * kernelSize / 2];
	auto zxy = im.at<uchar>(row, col);
	if (med > min && med < max)
	{
		// to B
		if (zxy > min && zxy < max)
			return zxy;
		else
			return med;
	}
	else
	{
		kernelSize += 2;
		if (kernelSize <= maxSize)
			return adaptiveProcess(im, row, col, kernelSize, maxSize); // ���󴰿ڳߴ磬����A���̡�
		else
			return med;
	}
}

int** on_trackbar(const char *SrcPath = "1.BMP"){

	CvSeq* contour = 0;
	CvSeq* _contour = contour;

	//����������Ķ�ά���飬����ָ������
	int** Feature_value2 = 0;
	Feature_value2 = new int*[value2.Feature_value2_number];

	IplImage *SrcImage_or;
	CvSize src_sz;
	////===============================================================================================Ԥ����
	//����ԭͼ
	printf("Ԥ����\n");
	IplImage *SrcImage_origin = cvLoadImage(SrcPath, CV_LOAD_IMAGE_UNCHANGED);

	//����	
	src_sz.width = SrcImage_origin->width* picture.scale;
	src_sz.height = SrcImage_origin->height* picture.scale;
	SrcImage_or = cvCreateImage(src_sz, SrcImage_origin->depth, SrcImage_origin->nChannels);
	cvResize(SrcImage_origin, SrcImage_or, CV_INTER_CUBIC);
	//cvNamedWindow("ԭͼ", 0);
	////��ʾԭͼ��ԭͼ����
	//cvShowImage("ԭͼ", SrcImage_or);

	//��ͨ���ҶȻ�����
	if (SrcImage_or->nChannels > 1)
	{
		g_GrayImage = cvCreateImage(cvSize(SrcImage_or->width, SrcImage_or->height), IPL_DEPTH_8U, 1);
		cvCvtColor(SrcImage_or, g_GrayImage, CV_BGR2GRAY);
	}
	else
		g_GrayImage = SrcImage_or;
	//�����ع����
	//IplImage *src_threshold = cvCreateImage(cvGetSize(SrcImage_or), IPL_DEPTH_8U, 1);
	//cvThreshold(SrcImage_or, src_threshold, 100, 255, CV_THRESH_BINARY);
	for (int i = 0; i < src_sz.height; i++)
	{
		for (int j = 0; j < src_sz.width; j++)
		{
			if (cvGet2D(g_GrayImage, i, j).val[0]>100)
				cvSet2D(g_GrayImage, i, j, 100);
		}
	}
	//cvNamedWindow("����", 0);
	//////��ʾԭͼ��ԭͼ����
	//cvShowImage("����", SrcImage_or);
	/// Ӧ��ֱ��ͼ���⻯
	IplImage *src_his = cvCreateImage(src_sz, g_GrayImage->depth, g_GrayImage->nChannels);
	cvEqualizeHist(g_GrayImage, src_his);
	cvSaveImage("���⻯.jpg",src_his);
	//fft�任
	//IplImage *Fourier = cvCreateImage(cvGetSize(src_his), IPL_DEPTH_64F, 2);
	//IplImage *dst = cvCreateImage(cvGetSize(src_his), IPL_DEPTH_64F, 2);
	//IplImage *ImageRe = cvCreateImage(cvGetSize(src_his), IPL_DEPTH_64F, 1);
	//IplImage *ImageIm = cvCreateImage(cvGetSize(src_his), IPL_DEPTH_64F, 1);
	//IplImage *Image = cvCreateImage(cvGetSize(src_his), src_his->depth, src_his->nChannels);
	//IplImage *ImageDst = cvCreateImage(cvGetSize(src_his), src_his->depth, src_his->nChannels);
	//double Minval, Maxval;
	//double scale;
	//double shift;
	//fft2(src_his, Fourier);                  //����Ҷ�任  
	//fft2shift(Fourier, Image);          //���Ļ�  
	//cvDFT(Fourier, dst, CV_DXT_INV_SCALE);//ʵ�ָ���Ҷ��任�����Խ����������  
	//cvSplit(dst, ImageRe, ImageIm, 0, 0);

	////������ÿ��Ԫ��ƽ�����洢�ڵڶ���������  
	//cvPow(ImageRe, ImageRe, 2);
	//cvPow(ImageIm, ImageIm, 2);
	//cvAdd(ImageRe, ImageIm, ImageRe, NULL);
	//cvPow(ImageRe, ImageRe, 0.5);
	//cvMinMaxLoc(ImageRe, &Minval, &Maxval, NULL, NULL);
	//scale = 255 / (Maxval - Minval);
	//shift = -Minval * scale;
	////��shift����ImageRe��Ԫ�ذ��������ŵĽ���ϣ��洢ΪImageDst  
	//cvConvertScale(ImageRe, src_his, scale, shift);
	//cvEqualizeHist(src_his, g_GrayImage);
	//cvAbsDiff(g_GrayImage, ImageRe, g_GrayImage);
	//cvNamedWindow("ԭͼ", CV_WINDOW_AUTOSIZE);
	////��ʾԭͼ��ԭͼ����
	//cvShowImage("ԭͼ", SrcImage);


	//������ֵ��ԭͼ
	printf("��ֵ��\n");
	g_BinaryImage = cvCreateImage(cvGetSize(g_GrayImage), IPL_DEPTH_8U, 1);

	cvThreshold(g_GrayImage, g_BinaryImage, picture.threshold_value_binaryzation, 255, CV_THRESH_BINARY);
	cvSaveImage("��ֵ��.jpg", g_BinaryImage);
	//��̬��ֵ
	//int blockSize = 7;
	//int constValue = 10;
	//cvAdaptiveThreshold(g_GrayImage, g_BinaryImage, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, blockSize, constValue);
	//��ʾ��ֵ�����ͼƬ
	//cvNamedWindow("��ֵ��", 0);
	//cvShowImage("��ֵ��", g_BinaryImage);
	//g_BinaryImage = cvCloneImage(g_BinaryImage);  //// ���͸�ʴ

	////===============================================================================================ͼ�����͸�ʴ
	//////��cvDilate��cvErode�������ͺ�ʴ�����Ϊ�պϲ�����ͼƬ�ж��Ѵ����ϡ�
	//////������������������ϸС�ն��������ٽ����壬ƽ�������Ե��ͬʱ�����Ըı��������
	printf("���͸�ʴ\n");
	IplImage* temp_cvDilate = cvCreateImage(cvGetSize(g_BinaryImage), IPL_DEPTH_8U, 1);
	IplImage* temp_cvErode = cvCreateImage(cvGetSize(g_BinaryImage), IPL_DEPTH_8U, 1);
	IplImage* temp_cvErode_cvErode = cvCreateImage(cvGetSize(g_BinaryImage), IPL_DEPTH_8U, 1);

	IplConvKernel * myModel1;
	myModel1 = cvCreateStructuringElementEx( //�Զ���5*5,�ο��㣨3,3���ľ���ģ��
		value.Model1_k1, value.Model1_k1, value.Model1_k2, value.Model1_k2, CV_SHAPE_ELLIPSE//CV_SHAPE_ELLIPSE, ��ԲԪ��;
		);
	IplConvKernel * myModel2;
	myModel2 = cvCreateStructuringElementEx( //�Զ���5*5,�ο��㣨3,3���ľ���ģ��
		value.Model2_k1, value.Model2_k1, value.Model2_k2, value.Model2_k2, CV_SHAPE_RECT	//CV_SHAPE_RECT, ������Ԫ��;
		);




	//////�����ͺ�ʴ
	cvDilate(g_BinaryImage, temp_cvDilate, myModel1, 1);//����
	cvErode(temp_cvDilate, temp_cvErode_cvErode, myModel2, 3);//��ʴ

	//namedWindow("temp_cvErode_cvErode", CV_WINDOW_AUTOSIZE);
	//cvShowImage("temp_cvErode_cvErode", temp_cvErode_cvErode);

	g_BinaryImage = cvCloneImage(temp_cvErode_cvErode);  //// �������͸�ʴ���


	///////================================================================================================�����ͨ����
	printf("�����ͨ����\n");
	CvMemStorage *liantong_storage = cvCreateMemStorage();
	IplImage* liantogn_dst = cvCreateImage(cvGetSize(g_BinaryImage), 8, 3);
	//��ȡ����   
	cvFindContours(g_BinaryImage, liantong_storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	cvZero(liantogn_dst);//�������   

	IplImage *result = cvCreateImage(cvSize(liantogn_dst->width, liantogn_dst->height), IPL_DEPTH_8U, 3);
	cvCvtColor(src_his, result, CV_GRAY2BGR);//��ת��RGBͼ�񣬷�����ɫ

	int n = -1, m = 0;//nΪ����������������mΪ��������   
	////-----------------------------------------------------------����ͨ����������
	for (; contour != 0; contour = contour->h_next)
	{

		double tmparea = fabs(cvContourArea(contour));
		
		if (tmparea <= value2.minarea)
		{
			cvSeqRemove(contour, 0); //ɾ�����С���趨ֵ������   
			continue;
		}
		else
		{
			liantong_all_area = liantong_all_area + tmparea;
		}

		CvRect aRect = cvBoundingRect(contour, 0);
		//if ((aRect.width / aRect.height)<1)
		//{
		//	cvSeqRemove(contour, 0); //ɾ����߱���С���趨ֵ������   
		//	continue;
		//}
		CvBox2D box=cvMinAreaRect2(contour);//��Ӿ���
		//printf("%d   %d  %f   %f  %f   %f  \n ", g_BinaryImage->width, g_BinaryImage->height, box.center.x, box.center.y, box.size.width, box.size.height);
		//if (box.center.x - box.size.width / 2 <= 0 || box.center.x + box.size.width / 2 >= g_BinaryImage->width ||
		//	box.center.y - box.size.height / 2 <= 0 || box.center.y + box.size.height / 2 >= g_BinaryImage->height)
		//{
		//	
		//	cvSeqRemove(contour, 0); //ɾ�������߽������   
		//	continue;
		//}

		CvScalar color = CV_RGB(rand() & 255, rand() & 255, rand() & 255);//�����ɫ
		//CvScalar color = CV_RGB(255,0,0);
		if (tmparea > value2.maxarea)
		{
			//value2.maxarea = tmparea;
			n = m;
			cvDrawContours(liantogn_dst, contour, color, color, -1, -1, 8);//�����ⲿ���ڲ������� 
			cvSeqRemove(contour, 0); //ɾ�������ͼ��  
			continue;
		}
		m++;

		cvDrawContours(liantogn_dst, contour, color, color, -1, -1, 8);//�����ⲿ���ڲ�������   
		cvDrawContours(result, contour, color, color, -1, -1, 8);//�����ⲿ���ڲ������� 
		//cvRectangle(src_his, CvPoint(box.center.y-box.size.height,box.center.x),
		//	CvPoint(box.center.y + box.size.height, box.center.x + box.size.width), 3, 4, 1);//�������겻̫�Ծ�����Ϊû���Ǿ��ο����ת
		//cvSaveImage("fanse.jpg", liantogn_dst);
	}
	//cvNamedWindow("�����", 0);
	//cvShowImage("�����", liantogn_dst);
	cvSaveImage("��ͨͼ.jpg", liantogn_dst);

	cvNamedWindow("�����", 0);
	cvShowImage("�����", result);
	cvSaveImage("jian.jpg", result);
	printf("����\n");
	return  Feature_value2; ////���ظ�����
}

//���ۼ��
void CheckScratch()
{
	Mat image, imagemen, diff, Mask;
	image = imread("C:\\Users\\lenovo\\Desktop\\img\\IMG_1725.BMP");
	//image = imread("F:\\workplace\\matlab_c\\matlab_c\\saveImage.jpg");

	//��ֵģ��
	printf("��˹�˲�\n");
	GaussianBlur(image, imagemen, Size(5, 5),0,0);

	//ͼ����
	printf("��ֲ���\n");
	subtract(imagemen, image, diff);

	//ͬ��̬��ֵ�ָ�dyn_threshold
	printf("��ֵ�ָ�\n");
	threshold(diff, Mask, 30, 255, THRESH_BINARY_INV);
	//cvNamedWindow("imagemean", 0);
	//imshow("imagemean", imagemen);
	//cvNamedWindow("diff", 0);
	//imshow("diff", diff);
	//cvNamedWindow("Mask", 0);
	//imshow("Mask", Mask);
	Mat imagegray;
	cvtColor(Mask, imagegray, CV_RGB2GRAY);
	vector<vector<Point>> contours;
	vector<Vec4i>hierarchy;
	
	findContours(imagegray, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	Mat drawing = Mat::zeros(Mask.size(), CV_8U);
	int j = 0;
	printf("contours\n");
	for (int i = 0; i < contours.size(); i++)
	{
		Moments moms = moments(Mat(contours[i]));
		double area = moms.m00;//��׾ؼ�Ϊ��ֵͼ������&nbsp; double area = moms.m00;��׾�.m00��ʾ�����������.m10Ϊ��������
		//�������������趨�ķ�Χ�����ٿ��Ǹðߵ�&nbsp;
		if (area > 100 && area < 1000000)
		{
			drawContours(drawing, contours, i, Scalar(255), FILLED, 8, hierarchy, 0, Point());
			j = j + 1;

		}
	}

	Mat element15(3, 3, CV_8U, Scalar::all(1));
	Mat close;
	morphologyEx(drawing, close, MORPH_CLOSE, element15);
	//cvNamedWindow("drawing", 0);
	//imshow("drawing", drawing);
	//waitKey(0);
	vector<vector<Point> > contours1;
	vector<Vec4i> hierarchy1;
	findContours(close, contours1, hierarchy1, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	//cvNamedWindow("close", 0);
	//imshow("close", close);
	//waitKey(0);
	j = 0;
	int m = 0;
	printf("contours num:%d\n", contours1.size());
	for (int i = 0; i < contours1.size(); i++)
	{
		Moments moms = moments(Mat(contours1[i]));
		double area = moms.m00;//��׾ؼ�Ϊ��ֵͼ������&nbsp; double area = moms.m00;
		//�������������趨�ķ�Χ�����ٿ��Ǹðߵ�&nbsp; 

		double area1 = contourArea(contours1[i]);
		drawContours(image, contours1, i, Scalar(0, 0, 255), FILLED, 80, hierarchy1, 0, Point());
		if (area > 100 && area < 1000000)
		{
			drawContours(image, contours1, i, Scalar(0, 0, 255), FILLED, 8, hierarchy1, 0, Point());
			j = j + 1;

		}
		else if (area >= 0 && area <= 50)
		{
			drawContours(image, contours1, i, Scalar(255, 0, 0), FILLED, 8, hierarchy1, 0, Point());
			m = m + 1;

		}
	}

	char t[256];
	sprintf_s(t, "%01d", j);
	string s = t;
	string txt = "Long NG : " + s;
	putText(image, txt, Point(20, 30), CV_FONT_HERSHEY_COMPLEX, 1,
		Scalar(0, 0, 255), 2, 8);

	sprintf_s(t, "%01d", m);
	s = t;
	txt = "Short NG : " + s;
	putText(image, txt, Point(20, 60), CV_FONT_HERSHEY_COMPLEX, 1,
		Scalar(255, 0, 0), 2, 8);
	imwrite("result.bmp", image);
	printf("finished");
	//cvDestroyWindow("imagemean");
	//cvDestroyWindow("diff");
	//cvDestroyWindow("Mask");
	//cvDestroyWindow("drawing");
	//cvDestroyWindow("close");
}


int main(){
	//rect_detector detector;
	//artifacts_detector artifacts_detector1;
	//mura_detector mura_detector1;
	//curve_detector curve_detector1;
	//cv::namedWindow("original");
	//cv::Mat img = cv::imread("C:\\Users\\lenovo\\Desktop\\img\\11.JPG");
	//cv::imshow("original", img);
	////cv::waitKey(1);
	////    mura_detector1.enable_debug();
	//artifacts_detector1.enable_debug();
	////    detector.enable_debug();
	////    curve_detector1.enable_debug();
	////cv::Mat scr = detector.detect_screen(img);
	//cv::Mat scr = img;
	//artifacts_detector1.detect_artifacts(img);
	//mura_detector1.detect_mura(scr);
	//curve_detector1.detect_curve(scr);
	//cv::imwrite("screen.jpg", scr);
	//cv::waitKey();
	//return 0;
	//CheckScratch();
	int **Tan_return;


	Tan_return = on_trackbar("1.BMP");
	cvWaitKey(0);


	////���ٴ��ڣ��ͷ�ͼƬ��ʵ�������˳�ʱһ��Ҫ���ٴ��ڣ�  
	//cvDestroyWindow(WindowBinaryTitle);  
	//cvDestroyWindow(WindowSrcTitle);  
	//cvReleaseImage(&g_BinaryImage);  
	//cvReleaseImage(&g_GrayImage);  
	//cvReleaseImage(&SrcImage);  
	getchar();
	return 0;
}

