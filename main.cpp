#include "stdafx.h"

#define GLEW_STATIC
#include <glew.h>
#include <gl/glut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <iomanip>
#include <fstream>

#include "technique.h"
#include "mesh.h"

void RenderScene(int ind,int cam_id);
Mesh *pMesh;
Technique *pShaderMgr;
#if 1
int width = 1600, height = 900;
float fx = height / 2, fy = height / 2, cx = width / 2, cy = height / 2;
#endif
#if 0
int width = 1148, height = 862;
float fx = width /2, fy = width /2,cx = width / 2, cy = height / 2;
#endif
GLuint textureColor;
GLuint textureDepth;
GLuint textureNormal;
glm::mat4 modelView;
glm::mat4 projection;
cv::Mat colorImg(height, width, CV_8UC3);
cv::Mat depthImg(height, width, CV_16UC1);
cv::Mat normalImg(height, width, CV_8UC3);

void RegisteredImgs(cv::Mat& colorDepthImg, cv::Mat& colorImg, cv::Mat& depthImg) {
	colorDepthImg.create(height, width, CV_8UC3);

	cv::Mat grayImg(height, width, CV_8UC1);
	cv::Mat grayDepthImg(height, width, CV_8UC1);
	depthImg.convertTo(grayDepthImg, CV_8UC1, 1.0f, 0);
	cv::cvtColor(colorImg, grayImg, CV_RGB2GRAY);
	for (int r = 0; r < colorDepthImg.rows; ++r)
	{
		for (int c = 0; c < colorDepthImg.cols; ++c)
		{
			cv::Vec3b& pixel = colorDepthImg.at<cv::Vec3b>(r, c);
			pixel[0] = 20;
			pixel[1] = grayImg.at<uchar>(r, c);
			pixel[2] = grayDepthImg.at<uchar>(r, c);
		}
	}
}

const static GLuint attachment_buffers[] = {
	GL_COLOR_ATTACHMENT0_EXT,
	GL_COLOR_ATTACHMENT1_EXT,
	GL_COLOR_ATTACHMENT2_EXT,
	GL_COLOR_ATTACHMENT3_EXT,
	GL_COLOR_ATTACHMENT4_EXT,
	GL_COLOR_ATTACHMENT5_EXT,
	GL_COLOR_ATTACHMENT6_EXT,
	GL_COLOR_ATTACHMENT7_EXT
};

int main(int argc, char **argv)
{
#if 0
	{
		/*float fovy = 90.0f / 180.0f * 3.1415926f;
		int width = 1280, height = 1024;
		float fy = height / 2.0f / tan(fovy / 2.0f);
		float fx = fy;
		float cx = (width - 1.0f) / 2.0f,
			cy = (height - 1.0f) / 2.0f;*/

		std::ifstream fs;
		//fs.open("G:\\xht\\ChairModel\\output\\camera_pose.txt", std::ifstream::binary);

		std::vector<glm::mat4> cameraPoseVec;
		for (int i = 0; i < 24; ++i)
		{
			fs.open("C:\\Users\\xht\\Desktop\\upload_TexOutput\\RT_%d.txt", i);
			glm::mat4 pose;
			fs.read((char *)&pose[0][0], sizeof(glm::mat4));
			cameraPoseVec.push_back(pose);
		}

		char colorDir[256];
		cv::Mat img;
		std::vector<float> vertices;
		float x, y, z;
		glm::vec4 v, v2;
		for (int i = 0; i < 24; i += 1)
		{
			//sprintf(colorDir, "G:\\xht\\ChairModel\\depth\\depth_%d.png", i);
			sprintf(colorDir, "G:\\xht\\ChairModel\\depth\\depth_%d.png", i);
			std::cout << colorDir << std::endl;
			img = cv::imread(colorDir, cv::IMREAD_ANYDEPTH);
			cv::imshow("img", img * 1);
			cv::waitKey(1);

			for (int row = 0; row < height; ++row)
			{
				for (int col = 0; col < width; ++col)
				{
					ushort depth = img.at<ushort>(row, col);
					if (depth != 0)
					{
						z = depth / 1000.0f;
						x = (col - cx) / fx * z;
						y = (row - cy) / fy * z;
						v.x = x;
						v.y = y;
						v.z = z;
						v.w = 1.0f;
						v2 = cameraPoseVec[i] * v;
						vertices.push_back(v2.x);
						vertices.push_back(v2.y);
						vertices.push_back(v2.z);
					}
				}
			}
		}
		fs.close();

		std::ofstream outfs;
		outfs.open("G:\\xht\\ChairModel\\test2.ply");

		outfs << "ply";
		outfs << "\nformat " << "ascii" << " 1.0";
		outfs << "\nelement vertex " << vertices.size() / 3;
		outfs << "\nproperty float x"
			"\nproperty float y"
			"\nproperty float z";
		outfs << "\nproperty uchar red"
			"\nproperty uchar green"
			"\nproperty uchar blue";
		outfs << "\nproperty float nx"
			"\nproperty float ny"
			"\nproperty float nz";
		//fs << "\nelement face " << Indices.size() / 3;
		//fs << "\nproperty list uchar int vertex_indices";
		outfs << "\nend_header\n";

		for (unsigned int i = 0; i < vertices.size() / 3; i++)
		{
			outfs << vertices[3 * i] << " " << vertices[3 * i + 1] << " " << vertices[3 * i + 2] << " "
				<< (int)255 << " " << (int)0 << " " << (int)0 << " "
				<< 1 << " " << 0 << " " << 0
				<< std::endl;
		}

		outfs.close();
		std::exit(0);
	}
#endif
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	//glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	glutInitWindowSize(width, height);
	glutCreateWindow("DepthTextureGenerator");
	GLenum res = glewInit();
	if (res != GLEW_OK) {
		std::cout << "error" << std::endl;
		return false;
	}

	pShaderMgr = new Technique();

	if (!pShaderMgr->Init()) {
		std::cout << "Error initializing the lighting technique" << std::endl;
		return false;
	}

	pMesh = new Mesh();

#if 1
	//pMesh->LoadMesh("C:\\Users\\xht\\Desktop\\upload_TexOutput\\input-model.obj", false);	
	pMesh->LoadMesh("G:\\xht\\huawei\\2019-04-25_15.49.41\\obj-origin\\textured.obj", false);
	//pMesh->LoadMesh("G:\\xht\\ChairModel\\Model.obj", false);
	//pMesh->LoadMesh("G:\\Sphere.obj", false);
#endif
#if 0
	pMesh->LoadMesh("E:\\Workspace\\DataSet\\XianlingDataSet\\input-model.stl", false);
#endif

	//std::exit(0);

	glGenTextures(1, &textureColor);
	glBindTexture(GL_TEXTURE_2D, textureColor);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glGenTextures(1, &textureDepth);
	glBindTexture(GL_TEXTURE_2D, textureDepth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE16UI_EXT, width, height, 0, GL_LUMINANCE_INTEGER_EXT, GL_UNSIGNED_SHORT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glGenTextures(1, &textureNormal);
	glBindTexture(GL_TEXTURE_2D, textureNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	GLuint fbo, rbo;
	glGenFramebuffersEXT(1, &fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, textureColor, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, textureDepth, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, textureNormal, 0);

	glGenRenderbuffersEXT(1, &rbo);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rbo);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, width, height);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, rbo);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, rbo);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

	pShaderMgr->Enable();

	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
#if 1
	for (int i = 15; i < 295; i++) {
		for (int cam_id = 0; cam_id < 2; cam_id++) {
			std::ifstream fs;
			char path[256];
			sprintf(path, "G:\\xht\\huawei\\2019-04-25_15.49.41\\scene\\cam\\%05d-cam%d.CAM", i,cam_id);
			fs.open(path);
			if(!fs)
				continue;
			std::cout << path << std::endl;
			modelView = glm::mat4(1.0f);
			for (int j = 0; j < 3; j++) {
				fs >> modelView[3][j];// 
			}
			for (int k = 0; k < 3; k++) {
				for (int j = 0; j < 3; j++) {
					fs >> modelView[j][k];// 
				}
			}
			modelView[0][1] *= -1;
			modelView[1][1] *= -1;
			modelView[2][1] *= -1;
			modelView[3][1] *= -1;
			for (int i = 0; i < 4; i++) {
				float t = modelView[i][1];
				modelView[i][1] = -modelView[i][0];
				modelView[i][0] = t;
			}
			modelView[3][2] *= -1;
			modelView[0][2] *= -1;
			modelView[1][2] *= -1;
			modelView[2][2] *= -1;
			//modelView = glm::mat4(1.0f);
			//modelView = glm::translate(modelView, glm::vec3(0.0f, 0.0f, -3.0f));
			////modelView = glm::rotate(modelView, glm::radians(-25.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			////modelView = glm::rotate(modelView, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));

			//modelView = glm::rotate(modelView, glm::radians(1.0f* i * 10), glm::vec3(0.0f, 1.0f, 0.0f));
			//modelView[3][2] *= -1;
			/*glm::vec3 rot = glm::vec3(modelView[0][0], modelView[0][1], modelView[0][2]);
			modelView = glm::rotate(modelView, glm::radians(180.0f),rot);*/
			fs.close();
			float far = 50.0f;
			float near = 0.1f;
			/*float L = -(cx)* near / fx;
			float T = -(cy)* near / fy;
			float R = (width - cx) * near / fx;
			float B = (height - cy) * near / fy;
			projection[0][0] = 2 * near / (R - L);
			projection[0][1] = 0.0f;
			projection[0][2] = 0.0f;
			projection[0][3] = 0.0f;

			projection[1][0] = 0.0f;
			projection[1][1] = 2 * near / (T - B);;
			projection[1][2] = 0.0f;
			projection[1][3] = 0.0f;

			projection[2][0] = -(R + L) / (R - L);
			projection[2][1] = -(T + B) / (T - B);
			projection[2][2] = (far + near) / (far - near);
			projection[2][3] = 1.0f;

			projection[3][0] = 0.0f;
			projection[3][1] = 0.0f;
			projection[3][2] = -2.0f * far * near / (far - near);
			projection[3][3] = 0.0f;*/
			projection[0][0] = 2 * fx / width;
			projection[0][1] = 0.0f;
			projection[0][2] = 0.0f;
			projection[0][3] = 0.0f;

			projection[1][0] = 0.0f;
			projection[1][1] = 2 * fy / height;
			projection[1][2] = 0.0f;
			projection[1][3] = 0.0f;

			projection[2][0] = (1.0f - 2 * cx / width);
			projection[2][1] = 2 * cy / height - 1.0f;
			projection[2][2] = -(far + near) / (far - near);
			projection[2][3] = -1.0f;

			projection[3][0] = 0.0f;
			projection[3][1] = 0.0f;
			projection[3][2] = -2.0f * far * near / (far - near);
			projection[3][3] = 0.0f;

			RenderScene(i,cam_id);
			//cv::Mat colorDepthImg, colorImgRead(height, width, CV_8UC3);
			//char colorPath[256];
			//sprintf(colorPath, "C:\\Users\\xht\\Desktop\\upload_TexOutput\\image\\rec_%d.jpg", i);
			//colorImgRead = cv::imread(colorPath, cv::IMREAD_COLOR);
			//RegisteredImgs(colorDepthImg, colorImgRead, depthImg);
			//cv::imshow("colorDepthImg", colorDepthImg);
			//cv::waitKey(0);
		}
	}
#endif
#if 0
	std::ofstream fs;
	fs.open("G:\\test\\camera_pose.txt" , std::ofstream::binary);
	int cameraNum = 36, cntcnt = 0;;
	float far = 1000.0f;
	float near = 0.1f;
	for (int i = 0; i < cameraNum; ++i)
	{
		modelView = glm::mat4(1.0f);
		modelView = glm::translate(modelView, glm::vec3(0.0f, 0.0f, -3.0f));
		//modelView = glm::rotate(modelView, glm::radians(-25.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		//modelView = glm::rotate(modelView, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		modelView = glm::rotate(modelView, glm::radians(1.0f*i*10), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 cameraPose = glm::inverse(modelView);

		cameraPose[1][0] *= -1;
		cameraPose[1][1] *= -1;
		cameraPose[1][2] *= -1;
		cameraPose[2][0] *= -1;
		cameraPose[2][1] *= -1;
		cameraPose[2][2] *= -1;

		fs.write((char *)&cameraPose[0][0], sizeof(glm::mat4));
		std::cout << "fragment: " << i << std::endl;
		projection = glm::perspective(glm::radians(90.0f), (float)width / (float)height, 1.0f, 50.0f);
		printf("proj: %f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n", projection[0][0], projection[0][1], projection[0][2], projection[0][3],
			projection[1][0], projection[1][1], projection[1][2], projection[1][3], projection[2][0], projection[2][1], projection[2][2], projection[2][3],
			projection[3][0], projection[3][1], projection[3][2], projection[3][3]);
		/*projection[0][0] = 2 * fx / width;
		projection[0][1] = 0.0f;
		projection[0][2] = 0.0f;
		projection[0][3] = 0.0f;

		projection[1][0] = 0.0f;
		projection[1][1] = 2 * fy / height;
		projection[1][2] = 0.0f;
		projection[1][3] = 0.0f;

		projection[2][0] = 1.0f - 2 * cx / width;
		projection[2][1] = 2 * cy / height - 1.0f;
		projection[2][2] = -(far + near) / (far - near);
		projection[2][3] = -1.0f;

		projection[3][0] = 0.0f;
		projection[3][1] = 0.0f;
		projection[3][2] = -2.0f * far * near / (far - near);
		projection[3][3] = 0.0f;*/
		RenderScene(cntcnt);
		++cntcnt;
		cv::waitKey(0);
	}
	fs.close();
#endif
	//modelView = glm::mat4(1.0f);
	//modelView = glm::translate(modelView, glm::vec3(2.0f, 0.0f, 0.0f));
	//std::cout << modelView[3][1] << std::endl;
	//modelView = glm::rotate(modelView, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	////projection = glm::perspective(glm::radians(60.0f), (float)width / (float)height, 1.0f, 50.0f);
	//float far = 50.0f;
	//float near = 0.1f;
	//projection[0][0] = 2 * fx / width;
	//projection[0][1] = 0.0f;
	//projection[0][2] = 0.0f;
	//projection[0][3] = 0.0f;

	//projection[1][0] = 0.0f;
	//projection[1][1] = 2 * fy / height;
	//projection[1][2] = 0.0f;
	//projection[1][3] = 0.0f;

	//projection[2][0] = 1.0f - 2 * cx / width;
	//projection[2][1] = 2 * cy / height - 1.0f;
	//projection[2][2] = -(far + near) / (far - near);
	//projection[2][3] = -1.0f;

	//projection[3][0] = 0.0f;
	//projection[3][1] = 0.0f;
	//projection[3][2] = -2.0f * far * near / (far - near);
	//projection[3][3] = 0.0f;
	/*RenderScene(0);
	cv::waitKey(0);*/
}

void RenderScene(int ind, int cam_id)
{
	glDrawBuffers(3, attachment_buffers);
	glPushAttrib(GL_VIEWPORT_BIT);
#if 1
	glViewport(0, 0, width, height);
#endif
#if 0
	glViewport(cx - width / 2.0, cy - height / 2.0, width, height);
#endif

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);

	glEnable(GL_NORMALIZE);
	//glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL); 

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

	pShaderMgr->SetModelView(modelView);
	pShaderMgr->SetProjection(projection);

	pMesh->Render();

	glPopAttrib();
	//glDrawBuffers(1, attachment_buffers);
	glFinish();	
	//glutSwapBuffers();

	glBindTexture(GL_TEXTURE_2D, textureColor);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, colorImg.data);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_2D, textureDepth);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_LUMINANCE_INTEGER_EXT, GL_UNSIGNED_SHORT, depthImg.data);
	glBindTexture(GL_TEXTURE_2D, 0);	

	glBindTexture(GL_TEXTURE_2D, textureNormal);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, normalImg.data);
	glBindTexture(GL_TEXTURE_2D, 0);

	std::vector<int> pngCompressionParams;
	pngCompressionParams.push_back(CV_IMWRITE_PNG_COMPRESSION);
	pngCompressionParams.push_back(0);
	//cv::namedWindow("depth");
	//cv::namedWindow("color");
	//cv::namedWindow("normal");
	//cv::namedWindow("mask");	
	/*cv::Vec3b rgb;
	bool flag = false;
	for (int v = 0; v < colorImg.rows; v++) {
		for (int u = 0; u < colorImg.cols; u++) {
			rgb = colorImg.at<cv::Vec3b>(v, u);
			if (rgb[0] != 0 || rgb[1] != 0 || rgb[2] != 0) {
				std::cout << "rgb: " << rgb << std::endl;
				flag = true;
				break;
			}
		}
		if (flag)
			break;
	}*/

	char colorDir[256], depthDir[256], normalDir[256];
	//sprintf(colorDir, "D:\\xjm\\xDepthTextureGenerator1.0\\output\\frame-%06d.color.png", ind);
	//sprintf(depthDir, "C:\\Users\\xht\\Desktop\\upload_TexOutput\\depth\\depth_%d.png", ind);
	//sprintf(depthDir, "G:\\test\\depth_%d.png", ind);
	sprintf(colorDir, "G:\\xht\\huawei\\colorImg\\2019-04-25_15.49.41\\color_%d_%d.png", ind,cam_id);
	//sprintf(normalDir, "D:\\xjm\\xDepthTextureGenerator1.0\\output\\frame-%06d.normal.png", ind);
#if 0
	sprintf(depthDir, "E:\\Workspace\\DataSet\\XianlingDataSet\\depth_imgs\\rec_%04d.png", ind + 1);
#endif

	//cv::Mat_<uchar> edgeDepth;
	//depthImg.convertTo(edgeDepth, CV_8UC1, 1.0 / 256.0);
	//cv::Canny(edgeDepth, edgeDepth, 5, 10);
	//cv::Mat element5(3, 3, CV_8U, cv::Scalar(1));
	//cv::Mat element5(5, 5, CV_8U, cv::Scalar(1));
	//cv::imshow("depth", edgeDepth);
	//cv::morphologyEx(edgeDepth, edgeDepth, cv::MORPH_CLOSE, element5);
	//cv::imshow("depth", edgeDepth);
	//cv::waitKey(0);

	//cv::dilate(edgeDepth, edgeDepth, element5);
	//cv::dilate(edgeDepth, edgeDepth, element5);
	//cv::imshow("depth", edgeDepth);
	//cv::waitKey(0);

#if 0
	int cnt = 0;
	for (int r = 0; r < edgeDepth.rows; ++r)
	{
		for (int c = 0; c < edgeDepth.cols; ++c)
		{
			if (edgeDepth(r, c) == 0)
			{
				++cnt;
				cv::floodFill(edgeDepth, cv::Point(c, r), cnt);
			}
		}
	}
	//cv::imshow("depth", edgeDepth * 60);
	//cv::waitKey(0);
	std::vector<int> nums(cnt + 1, 0);
	int val;
	for (int r = 0; r < edgeDepth.rows; ++r)
	{
		for (int c = 0; c < edgeDepth.cols; ++c)
		{
			val = edgeDepth(r, c);
			if (val >= 1 && val <= cnt)
			{
				++nums[val];
			}
		}
	}
	for (int i = 1; i <= cnt; ++i)
	{
		std::cout << nums[i] << ", ";
	}
	std::cout << std::endl;

	cv::Mat_<uchar> maskImg = cv::Mat_<uchar>::zeros(edgeDepth.rows, edgeDepth.cols);
	for (int r = 0; r < edgeDepth.rows; ++r)
	{
		for (int c = 0; c < edgeDepth.cols; ++c)
		{
			val = edgeDepth(r, c);
			if (val >= 1 && val <= cnt && nums[val] < 500)
			{
				maskImg(r, c) = 255;
			}
		}
	}
	//cv::imshow("depth", maskImg);
	//cv::waitKey(0);
	cv::dilate(maskImg, maskImg, element5);
	cv::dilate(maskImg, maskImg, element5);
	//cv::waitKey(0);

	for (int r = 0; r < depthImg.rows; ++r)
	{
		for (int c = 0; c < depthImg.cols; ++c)
		{
			if (maskImg(r, c) > 0)
			{
				depthImg.at<ushort>(r, c) = 0;
			}
		}
	}
#endif

#if 0
	for (int r = 0; r < edgeDepth.rows; ++r)
	{
		for (int c = 0; c < edgeDepth.cols; ++c)
		{
			if (edgeDepth(r, c) > 0)
			{
				depthImg.at<ushort>(r, c) = 0;
				colorImg.at<cv::Vec3b>(r, c) = cv::Vec3b(0, 0, 0);
				normalImg.at<cv::Vec3b>(r, c) = cv::Vec3b(0, 0, 0);
			}
		}
	}
#endif

    cv::flip(depthImg, depthImg, 0);
    cv::flip(colorImg, colorImg, 0);
   // cv::flip(normalImg, normalImg, 0);
	//cv::imshow("depth", depthImg * 10);
	cv::imshow("color", colorImg);
	//cv::imshow("normal", normalImg);
	//cv::imshow("mask", maskImg);

	cv::imwrite(colorDir, colorImg, pngCompressionParams);
	//cv::imwrite(depthDir, depthImg, pngCompressionParams);
	//cv::imwrite(normalDir, normalImg, pngCompressionParams);

	cv::waitKey(1000);

#if 0
	cv::Mat depthEdge, colorEdge;
	cv::Mat_<uchar> img8bit;
	depthImg.convertTo(img8bit, CV_8UC1, 1.0 / 1000.0f); // convert 16bit img to 8bit gray img, Note that we scale it !!!!!
	cv::imshow("depth", img8bit);
	cv::Canny(img8bit, depthEdge, 20, 40, 3);
	sprintf(colorDir, "E:\\Workspace\\DataSet\\XianlingDataSet\\rec_%04d.bmp", ind + 1);
	cv::Mat grayImg = cv::imread(colorDir, 0);
	cv::Canny(grayImg, colorEdge, 40, 100, 3);
	cv::Mat edgeImg = cv::Mat::zeros(colorEdge.rows, colorEdge.cols, CV_8UC3);
	for (int r = 0; r < edgeImg.rows; ++r)
	{
		for (int c = 0; c < edgeImg.rows; ++c)
		{
			edgeImg.at<cv::Vec3b>(r, c)[0] = colorEdge.at<uchar>(r, c);
			edgeImg.at<cv::Vec3b>(r, c)[2] = depthEdge.at<uchar>(r, c);
		}
	}
	
	cv::imshow("color", edgeImg);
	cv::waitKey(1);
#endif
}


