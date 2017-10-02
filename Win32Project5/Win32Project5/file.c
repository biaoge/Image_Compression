#include <stdio.h>
#include <math.h>
#include<Windows.h>
#include <commdlg.h>
#include"resource.h"
#include <stdlib.h>


typedef struct QuadTreeNode		//	用于获取图像信息的结点
{
    unsigned char uclr;    //颜色(灰度）
    int il;		//记录是否为叶子结点,0为非叶子结点，1为叶子结点
    int isx;	//像素的起始横坐标位置
    int isy;	//像素的起始纵坐标位置
    int ifx;	//像素的终止横坐标位置
    int ify;	//像素的终止纵坐标位置
    char ilev;		//树的层数
    struct QuadTreeNode *children[4];	//指向4个孩子结点的指针
    //struct QuadTreeNode *next;				//指向兄弟结点的指针
} QTree, *pQtre;

typedef struct		//用于保存图像信息的结点(当需要压缩的图像像素超过256X256)
{
    short ix;	//保存后的起始横坐标
    short iy;	//保存后的起始纵坐标
    unsigned char unclr;	//保存后的颜色（灰度）
    unsigned char islv;		//保存后的层数
} sQtre;

typedef struct		//用于保存图像信息的结点(当需要压缩的图像像素小于等于256X256)
{
	unsigned char ixm;		//保存后的起始横坐标
	unsigned char iym;		//保存后的起始纵坐标
	unsigned char unclrm;		//保存后的颜色（灰度）
	unsigned	char islvm;		//保存后的层数
}sQtrem;


//全局变量
unsigned char  *rpbmpbuf = NULL;		//读取图像指针
unsigned char  *wpbmpbuf = NULL;		//写入图像指针

//全局变量
int bmpWidth;		//图像宽度
int bmpHeight;		//图像高度
int lineByte;		//一行的像素值
char m=0;	//保存当前的层数
RGBQUAD *pColorTable;//颜色表指针
int biBitCount;//图像类型，每像素位数
int threshold_value = 0;//阀值大小
int number = 0;//记录节点个数
sQtre *fbuffer, *rbuffer;
sQtrem *fbufferm, *rbufferm;

static OPENFILENAME ofn ;
static TCHAR szFilter_C[] =     TEXT ("*.BMP\0*.bmp\0")  ;
static TCHAR szFilter_U[] =     TEXT ("*.DATA\0*.data\0")  ;

pQtre create_quadtree(int isx, int isy, int ifx, int ify, int lev);
void save_quad_tree(char *name, pQtre head);
void visit_quad_tree(pQtre head);
BOOL unload(char * name);


/***********************************************************************
* 函数名称：
*      init()
*函数参数：
*    无
*返回值：
*    无
*说明：初始化所有全局变量
***********************************************************************/
void init()
{
    rpbmpbuf = NULL;
    wpbmpbuf = NULL;
    bmpWidth = 0;
    bmpHeight = 0;
    lineByte = 0;
    pColorTable = NULL;
    biBitCount = 0;
    number = 0;//记录节点个数
    fbuffer = NULL;
    rbuffer = NULL;
}

void PopFileInitialize (HWND hwnd, TCHAR* SZFilter)

{
    ofn.lStructSize                      = sizeof (OPENFILENAME) ;
    ofn.hwndOwner                        = hwnd ;
    ofn.hInstance                        = NULL ;
    ofn.lpstrFilter                      = SZFilter ;
    ofn.lpstrCustomFilter = NULL ;
    ofn.nMaxCustFilter    = 0 ;
    ofn.nFilterIndex      = 0 ;
    ofn.lpstrFile         = NULL ;						// Set in Open and Close functions
    ofn.nMaxFile                = MAX_PATH ;
    ofn.lpstrFileTitle            = NULL ;              // Set in Open and Close functions
    ofn.nMaxFileTitle             = MAX_PATH ;
    ofn.lpstrInitialDir           = NULL ;
    ofn.lpstrTitle                = NULL ;
    ofn.Flags                    = 0 ;                  // Set in Open and Close functions
    ofn.nFileOffset               = 0 ;
    ofn.nFileExtension            = 0 ;
    ofn.lpstrDefExt               = NULL ;
    ofn.lCustData                 = 0L ;
    ofn.lpfnHook                  = NULL ;
    ofn.lpTemplateName            = NULL ;
}


BOOL PopFileUzipDlg (HWND hwnd, PTSTR pstrFileName, PTSTR pstrTitleName)
{
    int i;
    char pMBBuffer[MAX_PATH];
    PopFileInitialize (hwnd, szFilter_U);
    ofn.lpstrFile                 = pstrFileName ;
    ofn.lpstrFileTitle            = pstrTitleName ;
    ofn.Flags                    = OFN_HIDEREADONLY | OFN_CREATEPROMPT ;
    if(!GetOpenFileName (&ofn))
        return FALSE;
    wcstombs_s(&i, pMBBuffer, (size_t)MAX_PATH, pstrFileName, (size_t)MAX_PATH );
    if(!unload(pMBBuffer))
        return FALSE;
    for(i = 0;; i++)
        if(pMBBuffer[i] == '.')
            break;
    pMBBuffer[i++] = 'c';
    pMBBuffer[i++] = 'p';
    pMBBuffer[i++] = 'y';
    pMBBuffer[i++] = '.';
    pMBBuffer[i++] = 'b';
    pMBBuffer[i++] = 'm';
    pMBBuffer[i++] = 'p';
    pMBBuffer[i++] = '\0';

    saveBmp(pMBBuffer, wpbmpbuf, bmpWidth, bmpHeight, biBitCount, pColorTable);
    free(wpbmpbuf);
    wpbmpbuf = NULL;
    return TRUE;
}

BOOL PopFileComDlg (HWND hwnd, PTSTR pstrFileName, PTSTR pstrTitleName)
{
    int i;
    char pMBBuffer[MAX_PATH];
    pQtre head;
    PopFileInitialize (hwnd, szFilter_C);
    ofn.lpstrFile                 = pstrFileName ;
    ofn.lpstrFileTitle            = pstrTitleName ;
    ofn.Flags                    = OFN_HIDEREADONLY | OFN_CREATEPROMPT ;
    if(!GetOpenFileName (&ofn))
        return FALSE;
    wcstombs_s(&i, pMBBuffer, (size_t)MAX_PATH, pstrFileName, (size_t)MAX_PATH );
    readBmp(pMBBuffer);
    head = create_quadtree(0, 0, lineByte, bmpHeight,0);
    free(rpbmpbuf);
    rpbmpbuf = NULL;
    wpbmpbuf = (unsigned char*)malloc(sizeof(char)*lineByte*bmpHeight);
    for(i = 0; ; i++)
        if(pMBBuffer[i] == '.')
            break;
    pMBBuffer[++i] = 'd';
    pMBBuffer[++i] = 'a';
    pMBBuffer[++i] = 't';
    pMBBuffer[++i] = 'a';
    pMBBuffer[++i] = '\0';
    save_quad_tree(pMBBuffer, head);
    return TRUE;
}


/***********************************************************************
* 函数名称：
*      readBmp()
*函数参数：
*    char *bmpName -文件名字及路径
*返回值：
*    0为失败，1为成功
*说明：给定一个图像文件名及其路径，读图像的位图数据、宽、高、颜色表及每像素
*      位数等数据进内存，存放在相应的全局变量中
***********************************************************************/
BOOL readBmp(char *bmpName)
{
    BITMAPINFOHEADER head;
    FILE *fp;
    //二进制读方式打开指定的图像文件
    fp = fopen(bmpName,"rb");
    if(fp==0) return 0;
    //跳过位图文件头结构BITMAPFILEHEADER
    fseek(fp, sizeof(BITMAPFILEHEADER),0);
    //定义位图信息头结构变量，读取位图信息头进内存，存放在变量head中
    fread(&head, sizeof(BITMAPINFOHEADER), 1,fp);
    //获取图像宽、高、每像素所占位数等信息
    bmpWidth = head.biWidth;
    bmpHeight = head.biHeight;
    biBitCount = head.biBitCount;
    //定义变量，计算图像每行像素所占的字节数（必须是4的倍数）
    lineByte=(bmpWidth * biBitCount/8+3)/4*4;
    //灰度图像有颜色表，且颜色表表项为256
    if(biBitCount==8)
    {
        pColorTable = (RGBQUAD*)malloc(256*sizeof(RGBQUAD));//申请存储空间
        fread(pColorTable,sizeof(RGBQUAD),256,fp);//读颜色表进内存
    }
    //申请位图数据所需要的空间，读位图数据进内存
    rpbmpbuf = (unsigned char*)malloc(bmpHeight*lineByte*sizeof(char));
    fread(rpbmpbuf, 1, bmpHeight*lineByte, fp);
    //关闭文件
    fclose(fp);
    return 1;
}


/*****************************************
* 函数名称：
*      saveBmp()
*函数参数：
*    char *bmpName-文件名字及路径
*    unsigned char *imgBuf-待存盘的位图数据
*    int width-以像素为单位待存盘位图的宽
*    int height-以像素为单位待存盘位图高
*    int biBitCount-每像素所占位数
*    RGBQUAD *pColorTable-颜色表指针
*返回值：
*    0为失败，1为成功
*说明：给定一个图像位图数据、宽、高、颜色表指针及每像素所占的位数等信息，
*      将其写到指定文件中
***********************************************************************/
BOOL saveBmp(char *bmpName, unsigned char *imgBuf, int width, int height,
             int biBitCount, RGBQUAD *pColorTable)
{
    int colorTablesize;
    int lineByte;
    FILE *fp;
    BITMAPFILEHEADER fileHead;
    BITMAPINFOHEADER head;
    //如果位图数据指针为0，则没有数据传入，函数返回
    if(!imgBuf)
        return 0;
    //颜色表大小，以字节为单位，灰度图像颜色表为1024字节，彩色图像颜色表大小为0
    colorTablesize=0;
    if(biBitCount==8)
        colorTablesize=1024;
    //待存储图像数据每行字节数为4的倍数
    lineByte=(width * biBitCount/8+3)/4*4;
    //以二进制写的方式打开文件
    fp=fopen(bmpName,"wb");
    if(fp==0) return 0;
    //申请位图文件头结构变量，填写文件头信息
    fileHead.bfType = 0x4D42;//bmp类型
    //bfSize是图像文件4个组成部分之和
    fileHead.bfSize= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)
                     + colorTablesize + lineByte*height;
    fileHead.bfReserved1 = 0;
    fileHead.bfReserved2 = 0;
    //bfOffBits是图像文件前3个部分所需空间之和
    fileHead.bfOffBits=54+colorTablesize;
    //写文件头进文件
    fwrite(&fileHead, sizeof(BITMAPFILEHEADER),1, fp);
    //申请位图信息头结构变量，填写信息头信息
    head.biBitCount=biBitCount;
    head.biClrImportant=0;
    head.biClrUsed=0;
    head.biCompression=0;
    head.biHeight=height;
    head.biPlanes=1;
    head.biSize=40;
    head.biSizeImage=lineByte*height;
    head.biWidth=width;
    head.biXPelsPerMeter=0;
    head.biYPelsPerMeter=0;
    //写位图信息头进内存
    fwrite(&head, sizeof(BITMAPINFOHEADER),1, fp);
    //如果灰度图像，有颜色表，写入文件
    if(biBitCount==8)
        fwrite(pColorTable, sizeof(RGBQUAD),256, fp);
    //写位图数据进文件
    fwrite(imgBuf, height*lineByte, 1, fp);
    //关闭文件
    fclose(fp);
    return 1;
}


/*****************************************
* 函数名称：
*      average()
*函数参数：
*    int isx-图像起始横坐标
*    int isy-图形起始纵坐标
*    int ifx-图像终止横坐标
*    int ify-图像终止纵坐标
*返回值：
*    int型iave平均值
*说明：对一矩形区内的位图数据取平均值
***********************************************************************/
int average(int isx, int isy, int ifx, int ify)
{
    int i,j;
    int iave;
    int iamt=0;
    for(i=isy; i<ify; i++)
        for ( j=isx; j<ifx; j++)
            iamt+=rpbmpbuf[i*lineByte+j];
    if(isx==ifx || isy==ify)
        iave=rpbmpbuf[ify*lineByte+ifx];
    else
        iave=iamt/((ifx-isx)*(ify-isy));
    return iave;
}


/*****************************************
* 函数名称：
*      create_quadtree()
*函数参数：
*    int isx-图像起始横坐标
*    int isy-图形起始纵坐标
*    int ifx-图像终止横坐标
*    int ify-图像终止纵坐标
*返回值：
*    pQtre型头指针
*说明：利用空间四叉树技术对一副位图形进行四分，并建立四叉树,存放区域位置和颜色信息
***********************************************************************/
pQtre create_quadtree(int isx, int isy, int ifx, int ify, int lev)
{
    pQtre head;
    int i,j;
    int ave;
    int k=0;			//是否需要创建四叉树
    head=(pQtre)malloc(sizeof(QTree));
    head->il=0;		//置为非叶子结点
    lev++;			//	层数增加1
    ave = average(isx, isy, ifx, ify);//计算该区域内灰度值的均值
    for(i = isy; i< ify; i++)
    {
        for(j = isx; j< ifx; j++)
            if(abs(rpbmpbuf[i *lineByte + j ] - ave) > threshold_value)
            {
                k = 1;
                break;
            }
        if(k)
            break;
    }
    if(k==0)		//k == 0 递归出口
    {
        head->uclr = ave;//获取灰度值
        //获取区域位置
        head->isx = isx;
        head->isy = isy;
        head->ifx = ifx;
        head->ify = ify;
        head->il = 1;//改为叶子节点;
        number++;
        head->ilev=lev;	//获取结点层数
        head->children[0] = NULL;
        head->children[1] = NULL;
        head->children[2] = NULL;
        head->children[3] = NULL;
    }
    else
    {
        head->children[0] = create_quadtree(isx, isy, (isx + ifx)/2, (isy + ify)/2, lev);//递归建立左左子树 (左上)
        head->children[1] = create_quadtree((isx + ifx)/2, isy, ifx, (isy + ify)/2, lev);//递归建立左右子树 (右上)
        head->children[2] = create_quadtree((isx + ifx)/2, (isy + ify)/2, ifx, ify, lev);//递归建立右左子树 (左下)
        head->children[3] = create_quadtree(isx, (isy + ify)/2, (isx + ifx)/2, ify, lev);//递归建立右右子树（右下）
    }
    return head;
}


/*****************************************
* 函数名称：
*      visit_quad_tree()
*函数参数：
*    pQtre head-四叉树的头指针
*返回值：
*    无
*说明：先序遍历四叉树的叶子结点，并将其信息存入到fbuffer中
***********************************************************************/
void visit_quad_tree(pQtre head)
{
    if(!head)
        return;
    if(head->il)
    {
        fbuffer->unclr=head->uclr;
        fbuffer->ix=head->isx;
        fbuffer->iy=head->isy;
        fbuffer->islv=head->ilev;
        fbuffer++;
    }
    visit_quad_tree(head->children[0]);
    visit_quad_tree(head->children[1]);
    visit_quad_tree(head->children[2]);
    visit_quad_tree(head->children[3]);
    return ;
}


/*****************************************
* 函数名称：
*      visit_quad_treem()
*函数参数：
*    pQtre head-四叉树的头指针
*返回值：
*    无
*说明：先序遍历四叉树的叶子结点，并将其信息存入到fbufferm中
***********************************************************************/
void visit_quad_treem(pQtre head)
{
    if(!head)
        return;
    if(head->il)
    {
        fbufferm->unclrm=head->uclr;
        fbufferm->ixm=head->isx;
        fbufferm->iym=head->isy;
        fbufferm->islvm=head->ilev;
        fbufferm++;
    }
    visit_quad_treem(head->children[0]);
    visit_quad_treem(head->children[1]);
    visit_quad_treem(head->children[2]);
    visit_quad_treem(head->children[3]);
    return ;
}

/*****************************************
* 函数名称：
*      save_quad_tree()
*函数参数：
*    char *name-要存放图像信息的文件名
*	 pQtre head-获取了图像信息的四叉树头结点
*返回值：
*    无
*说明：先序遍历四叉树的叶子结点，判断分辨率是否大于256X256，并将其信息存入到对应内存中
***********************************************************************/
void save_quad_tree(char *name, pQtre head)
{
    int total=number;
    FILE *fil=fopen(name,"wb");
	if(bmpHeight<=256 && bmpWidth<=256)
	{
		rbufferm=fbufferm=(sQtrem *)malloc(number*sizeof(sQtrem));
		visit_quad_treem(head);
		fwrite(&biBitCount, sizeof(int), 1,fil);
		fwrite(&bmpWidth, sizeof(int), 1,fil);
		fwrite(&bmpHeight, sizeof(int), 1,fil);
		if(biBitCount==8)
			fwrite(pColorTable, sizeof(RGBQUAD),256, fil);
		fwrite(&total,sizeof(int),1,fil);
		fwrite(rbufferm,sizeof(sQtrem),total,fil);
		fclose(fil);
		return ;
		
	}
	else
	{
		rbuffer=fbuffer=(sQtre *)malloc(number*sizeof(sQtre));
		visit_quad_tree(head);
		fwrite(&biBitCount, sizeof(int), 1,fil);
		fwrite(&bmpWidth, sizeof(int), 1,fil);
		fwrite(&bmpHeight, sizeof(int), 1,fil);
		if(biBitCount==8)
			fwrite(pColorTable, sizeof(RGBQUAD),256, fil);
		fwrite(&total,sizeof(int),1,fil);
		fwrite(rbuffer,sizeof(sQtre),total,fil);
		fclose(fil);
		return ;
	}
}


/***********************************************************************
* 函数名称：
*      unload()
*函数参数：
*    char *name -文件名字
*返回值：
*    0为失败，1为成功
*说明：根据所给文件名字，及其包含的分辨率信息大小，对文件进行解压缩，恢复像素点的灰度信息
***********************************************************************/
BOOL unload(char * name)
{
    FILE * fil;
    int total;
    sQtre *buffer;
	sQtrem *bufferm;
    int i,j;
    int x,y;
    int deltax,deltay;
    int k,p;
    fil=fopen(name,"rb");
    if(!fil)
    {
        return  FALSE;
    }
    fread(&biBitCount, sizeof(int), 1,fil);
    fread(&bmpWidth, sizeof(int), 1,fil);
    fread(&bmpHeight, sizeof(int), 1,fil);
    if(biBitCount==8)
        pColorTable = (RGBQUAD*)malloc(256*sizeof(RGBQUAD));//申请存储空间
    fread(pColorTable, sizeof(RGBQUAD),256, fil);
    //定义变量，计算图像每行像素所占的字节数（必须是4的倍数）
    lineByte=(bmpWidth * biBitCount/8+3)/4*4;
    wpbmpbuf=(unsigned char *)malloc(lineByte*bmpHeight*sizeof(char *));
    fread(&total,sizeof(int),1,fil);
	if(bmpHeight<=256 && bmpWidth <=256)
	{
		bufferm=(sQtrem *)malloc(total*sizeof(sQtrem));
		fread(bufferm,sizeof(sQtrem),total,fil);
		for(k=0; k<total; k++)
		{
			x=bufferm[k].ixm;
			y=bufferm[k].iym;
			deltax=bmpWidth;
			deltay=bmpHeight;
			for(p=0; p<(bufferm[k].islvm-1); p++)
			{
				deltax=deltax/2;
				deltay=deltay/2;
			}
			for(i=y; i<y+deltay; i++)
				for(j=x; j<x+deltax; j++)
					wpbmpbuf[i*lineByte+j]=bufferm[k].unclrm;
		}
	}
	else
	{
		buffer=(sQtre *)malloc(total*sizeof(sQtre));
		fread(buffer,sizeof(sQtre),total,fil);
		for(k=0; k<total; k++)
		{
			x=buffer[k].ix;
			y=buffer[k].iy;
			deltax=bmpWidth;
			deltay=bmpHeight;
			for(p=0; p<(buffer[k].islv-1); p++)
			{
				deltax=deltax/2;
				deltay=deltay/2;
			}
			for(i=y; i<y+deltay; i++)
				for(j=x; j<x+deltax; j++)
					wpbmpbuf[i*lineByte+j]=buffer[k].unclr;
		}
	}
    return TRUE;
}
