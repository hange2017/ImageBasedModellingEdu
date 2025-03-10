//Created by sway on 2018/8/29.
   /* 测试8点法求取基础矩阵F
    *
    * [直接线性变换法]
    * 双目视觉中相机之间存在对极约束
    *
    *                       p2'Fp1=0,
    *
    * 其中p1, p2 为来自两个视角的匹配对的归一化坐标，并表示成齐次坐标形式，
    * 即p1=[x1, y1, z1]', p2=[x2, y2, z2],将p1, p2的表达形式带入到
    * 上式中，可以得到如下表达形式
    *
    *          [x2] [f11, f12, f13] [x1, y1, z1]
    *          [y2] [f21, f22, f23]                = 0
    *          [z2] [f31, f32, f33]
    *
    * 进一步可以得到
    * x1*x2*f11 + x2*y1*f12 + x2*f13 + x1*y2*f21 + y1*y2*f22 + y2*f23 + x1*f31 + y1*f32 + f33=0
    *
    * 写成向量形式
    *               [x1*x2, x2*y1,x2, x1*y2, y1*y2, y2, x1, y1, 1]*f = 0,
    * 其中f=[f11, f12, f13, f21, f22, f23, f31, f32, f33]'
    *
    * 由于F无法确定尺度(up to scale, 回想一下三维重建是无法确定场景真实尺度的)，因此F秩为8，
    * 这意味着至少需要8对匹配对才能求的f的解。当刚好有8对点时，称为8点法。当匹配对大于8时需要用最小二乘法进行求解
    *
    *   [x11*x12, x12*y11,x12, x11*y12, y11*y12, y12, x11, y11, 1]
    *   [x21*x22, x22*y21,x22, x21*y22, y21*y22, y22, x21, y21, 1]
    *   [x31*x32, x32*y31,x32, x31*y32, y31*y32, y32, x31, y31, 1]
    * A=[x41*x42, x42*y41,x42, x41*y42, y41*y42, y42, x41, y41, 1]
    *   [x51*x52, x52*y51,x52, x51*y52, y51*y52, y52, x51, y51, 1]
    *   [x61*x62, x62*y61,x62, x61*y62, y61*y62, y62, x61, y61, 1]
    *   [x71*x72, x72*y71,x72, x71*y72, y71*y72, y72, x71, y71, 1]
    *   [x81*x82, x82*y81,x82, x81*y22, y81*y82, y82, x81, y81, 1]
    *
    *现在任务变成了求解线性方程
    *               Af = 0
    *（该方程与min||Af||, subject to ||f||=1 等价)
    *通常的解法是对A进行SVD分解，取最小奇异值对应的奇异向量作为f分解
    *
    *本项目中对矩阵A的svd分解并获取其最小奇异值对应的奇异向量的代码为
    *   math::Matrix<double, 9, 9> V;
    *   math::matrix_svd<double, 8, 9>(A, nullptr, nullptr, &V);
    *   math::Vector<double, 9> f = V.col(8);
    *
    *
    *[奇异性约束]
    *  基础矩阵F的一个重要的性质是F是奇异的，秩为2，因此有一个奇异值为0。通过上述直接线性法求得
    *  矩阵不具有奇异性约束。常用的方法是将求得得矩阵投影到满足奇异约束得空间中。
    *  具体地，对F进行奇异值分解
    *               F = USV'
    *  其中S是对角矩阵，S=diag[sigma1, sigma2, sigma3]
    *  将sigma3设置为0，并重构F
    *                       [sigma1, 0,     ,0]
    *                 F = U [  0   , sigma2 ,0] V'
    *                       [  0   , 0      ,0]
    */

#include <math/matrix_svd.h>
#include "math/matrix.h"
#include "math/vector.h"

typedef math::Matrix<double, 3, 3>  FundamentalMatrix;

FundamentalMatrix fundamental_8_point (math::Matrix<double, 3, 8> const& points1
                         , math::Matrix<double, 3, 8> const& points2
                        ){

    FundamentalMatrix F;
    /** TODO HERE
     * Coding Here!!
     */
    //坐标已经是归一化平面上的点
    math::Matrix<double,8,9> data_mat;
    for(int i=0;i<8;i++){
        data_mat(i,0) = points2.col(i)[0]*points1.col(i)[0];
        data_mat(i,1) = points2.col(i)[0]*points1.col(i)[1];
        data_mat(i,2) = points2.col(i)[0]*points1.col(i)[2];
        data_mat(i,3) = points2.col(i)[1]*points1.col(i)[0];
        data_mat(i,4) = points2.col(i)[1]*points1.col(i)[1];
        data_mat(i,5) = points2.col(i)[1]*points1.col(i)[2];
        data_mat(i,6) = points2.col(i)[2]*points1.col(i)[0];
        data_mat(i,7) = points2.col(i)[2]*points1.col(i)[1];
        data_mat(i,8) = points2.col(i)[2]*points1.col(i)[2];
    }

    //SVD求解AX=0的直接线性解
    math::Matrix<double,8,9> U_data_mat;
    math::Matrix<double,9,9> S_data_mat,V_data_mat;
    math::matrix_svd<double,8,9>(data_mat,&U_data_mat,&S_data_mat,&V_data_mat);

    math::Vector<double,9> f_vec=V_data_mat.col(8);
    F(0,0) = f_vec[0];F(0,1) = f_vec[1];F(0,2) = f_vec(2);
    F(1,0) = f_vec[3];F(1,1) = f_vec[4];F(1,2) = f_vec(5);
    F(2,0) = f_vec[6];F(2,1) = f_vec[7];F(2,2) = f_vec(8);

    //对F按照特征值约束进行重构
    math::Matrix<double,3,3> U_F,S_F,V_F;
    math::matrix_svd<double,3,3>(F,&U_F,&S_F,&V_F);
    S_F(2,2) = 0;//约束
    F = U_F*S_F*V_F.transpose();//重构

    //SVD分解
    return F;
#if 0
    /* direct linear transform */
    math::Matrix<double, 8, 9> A;
    for(int i=0; i<8; i++)
    {
        math::Vec3d p1  = points1.col(i);
        math::Vec3d p2 = points2.col(i);

        A(i, 0) = p1[0]*p2[0];
        A(i, 1) = p1[1]*p2[0];
        A(i, 2) = p2[0];
        A(i, 3) = p1[0]*p2[1];
        A(i, 4) = p1[1]*p2[1];
        A(i, 5) = p2[1];
        A(i, 6) = p1[0];
        A(i, 7) = p1[1];
        A(i, 8) = 1.0;
    }

    math::Matrix<double, 9, 9> vv;
    math::matrix_svd<double, 8, 9>(A, nullptr, nullptr, &vv);
    math::Vector<double, 9> f = vv.col(8);

    FundamentalMatrix F;
    F(0,0) = f[0]; F(0,1) = f[1]; F(0,2) = f[2];
    F(1,0) = f[3]; F(1,1) = f[4]; F(1,2) = f[5];
    F(2,0) = f[6]; F(2,1) = f[7]; F(2,2) = f[8];

    /* singularity constraint */
    math::Matrix<double, 3, 3> U, S, V;
    math::matrix_svd(F, &U, &S, &V);
    S(2,2)=0;
    F = U*S*V.transpose();

    return F;
#endif

}

int main(int argc, char*argv[])
{

    // 第一幅图像中的对应点
    math::Matrix<double, 3, 8> pset1;
    pset1(0, 0) = 0.180123 ; pset1(1, 0)= -0.156584; pset1(2, 0)=1.0;
    pset1(0, 1) = 0.291429 ; pset1(1, 1)= 0.137662 ; pset1(2, 1)=1.0;
    pset1(0, 2) = -0.170373; pset1(1, 2)= 0.0779329; pset1(2, 2)=1.0;
    pset1(0, 3) = 0.235952 ; pset1(1, 3)= -0.164956; pset1(2, 3)=1.0;
    pset1(0, 4) = 0.142122 ; pset1(1, 4)= -0.216048; pset1(2, 4)=1.0;
    pset1(0, 5) = -0.463158; pset1(1, 5)= -0.132632; pset1(2, 5)=1.0;
    pset1(0, 6) = 0.0801864; pset1(1, 6)= 0.0236417; pset1(2, 6)=1.0;
    pset1(0, 7) = -0.179068; pset1(1, 7)= 0.0837119; pset1(2, 7)=1.0;
    //第二幅图像中的对应
    math::Matrix<double, 3, 8> pset2;
    pset2(0, 0) = 0.208264 ; pset2(1, 0)= -0.035405 ; pset2(2, 0) = 1.0;
    pset2(0, 1) = 0.314848 ; pset2(1, 1)=  0.267849 ; pset2(2, 1) = 1.0;
    pset2(0, 2) = -0.144499; pset2(1, 2)= 0.190208  ; pset2(2, 2) = 1.0;
    pset2(0, 3) = 0.264461 ; pset2(1, 3)= -0.0404422; pset2(2, 3) = 1.0;
    pset2(0, 4) = 0.171033 ; pset2(1, 4)= -0.0961747; pset2(2, 4) = 1.0;
    pset2(0, 5) = -0.427861; pset2(1, 5)= 0.00896567; pset2(2, 5) = 1.0;
    pset2(0, 6) = 0.105406 ; pset2(1, 6)= 0.140966  ; pset2(2, 6) = 1.0;
    pset2(0, 7) =  -0.15257; pset2(1, 7)= 0.19645   ; pset2(2, 7) = 1.0;

    FundamentalMatrix F = fundamental_8_point(pset1, pset2);


    std::cout<<"Fundamental matrix after singularity constraint is:\n "<<F<<std::endl;

    std::cout<<"Result should be: \n"<<"-0.0315082 -0.63238 0.16121\n"
                                     <<"0.653176 -0.0405703 0.21148\n"
                                     <<"-0.248026 -0.194965 -0.0234573\n" <<std::endl;

    return 0;
}