#include <stdio.h>
#include <math.h>
#include<Windows.h>
#include <commdlg.h>
#include"resource.h"
#include <stdlib.h>


typedef struct QuadTreeNode		//	���ڻ�ȡͼ����Ϣ�Ľ��
{
    unsigned char uclr;    //��ɫ(�Ҷȣ�
    int il;		//��¼�Ƿ�ΪҶ�ӽ��,0Ϊ��Ҷ�ӽ�㣬1ΪҶ�ӽ��
    int isx;	//���ص���ʼ������λ��
    int isy;	//���ص���ʼ������λ��
    int ifx;	//���ص���ֹ������λ��
    int ify;	//���ص���ֹ������λ��
    char ilev;		//���Ĳ���
    struct QuadTreeNode *children[4];	//ָ��4�����ӽ���ָ��
    //struct QuadTreeNode *next;				//ָ���ֵܽ���ָ��
} QTree, *pQtre;

typedef struct		//���ڱ���ͼ����Ϣ�Ľ��(����Ҫѹ����ͼ�����س���256X256)
{
    short ix;	//��������ʼ������
    short iy;	//��������ʼ������
    unsigned char unclr;	//��������ɫ���Ҷȣ�
    unsigned char islv;		//�����Ĳ���
} sQtre;

typedef struct		//���ڱ���ͼ����Ϣ�Ľ��(����Ҫѹ����ͼ������С�ڵ���256X256)
{
	unsigned char ixm;		//��������ʼ������
	unsigned char iym;		//��������ʼ������
	unsigned char unclrm;		//��������ɫ���Ҷȣ�
	unsigned	char islvm;		//�����Ĳ���
}sQtrem;


//ȫ�ֱ���
unsigned char  *rpbmpbuf = NULL;		//��ȡͼ��ָ��
unsigned char  *wpbmpbuf = NULL;		//д��ͼ��ָ��

//ȫ�ֱ���
int bmpWidth;		//ͼ����
int bmpHeight;		//ͼ��߶�
int lineByte;		//һ�е�����ֵ
char m=0;	//���浱ǰ�Ĳ���
RGBQUAD *pColorTable;//��ɫ��ָ��
int biBitCount;//ͼ�����ͣ�ÿ����λ��
int threshold_value = 0;//��ֵ��С
int number = 0;//��¼�ڵ����
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
* �������ƣ�
*      init()
*����������
*    ��
*����ֵ��
*    ��
*˵������ʼ������ȫ�ֱ���
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
    number = 0;//��¼�ڵ����
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
* �������ƣ�
*      readBmp()
*����������
*    char *bmpName -�ļ����ּ�·��
*����ֵ��
*    0Ϊʧ�ܣ�1Ϊ�ɹ�
*˵��������һ��ͼ���ļ�������·������ͼ���λͼ���ݡ����ߡ���ɫ��ÿ����
*      λ�������ݽ��ڴ棬�������Ӧ��ȫ�ֱ�����
***********************************************************************/
BOOL readBmp(char *bmpName)
{
    BITMAPINFOHEADER head;
    FILE *fp;
    //�����ƶ���ʽ��ָ����ͼ���ļ�
    fp = fopen(bmpName,"rb");
    if(fp==0) return 0;
    //����λͼ�ļ�ͷ�ṹBITMAPFILEHEADER
    fseek(fp, sizeof(BITMAPFILEHEADER),0);
    //����λͼ��Ϣͷ�ṹ��������ȡλͼ��Ϣͷ���ڴ棬����ڱ���head��
    fread(&head, sizeof(BITMAPINFOHEADER), 1,fp);
    //��ȡͼ����ߡ�ÿ������ռλ������Ϣ
    bmpWidth = head.biWidth;
    bmpHeight = head.biHeight;
    biBitCount = head.biBitCount;
    //�������������ͼ��ÿ��������ռ���ֽ�����������4�ı�����
    lineByte=(bmpWidth * biBitCount/8+3)/4*4;
    //�Ҷ�ͼ������ɫ������ɫ�����Ϊ256
    if(biBitCount==8)
    {
        pColorTable = (RGBQUAD*)malloc(256*sizeof(RGBQUAD));//����洢�ռ�
        fread(pColorTable,sizeof(RGBQUAD),256,fp);//����ɫ����ڴ�
    }
    //����λͼ��������Ҫ�Ŀռ䣬��λͼ���ݽ��ڴ�
    rpbmpbuf = (unsigned char*)malloc(bmpHeight*lineByte*sizeof(char));
    fread(rpbmpbuf, 1, bmpHeight*lineByte, fp);
    //�ر��ļ�
    fclose(fp);
    return 1;
}


/*****************************************
* �������ƣ�
*      saveBmp()
*����������
*    char *bmpName-�ļ����ּ�·��
*    unsigned char *imgBuf-�����̵�λͼ����
*    int width-������Ϊ��λ������λͼ�Ŀ�
*    int height-������Ϊ��λ������λͼ��
*    int biBitCount-ÿ������ռλ��
*    RGBQUAD *pColorTable-��ɫ��ָ��
*����ֵ��
*    0Ϊʧ�ܣ�1Ϊ�ɹ�
*˵��������һ��ͼ��λͼ���ݡ����ߡ���ɫ��ָ�뼰ÿ������ռ��λ������Ϣ��
*      ����д��ָ���ļ���
***********************************************************************/
BOOL saveBmp(char *bmpName, unsigned char *imgBuf, int width, int height,
             int biBitCount, RGBQUAD *pColorTable)
{
    int colorTablesize;
    int lineByte;
    FILE *fp;
    BITMAPFILEHEADER fileHead;
    BITMAPINFOHEADER head;
    //���λͼ����ָ��Ϊ0����û�����ݴ��룬��������
    if(!imgBuf)
        return 0;
    //��ɫ���С�����ֽ�Ϊ��λ���Ҷ�ͼ����ɫ��Ϊ1024�ֽڣ���ɫͼ����ɫ���СΪ0
    colorTablesize=0;
    if(biBitCount==8)
        colorTablesize=1024;
    //���洢ͼ������ÿ���ֽ���Ϊ4�ı���
    lineByte=(width * biBitCount/8+3)/4*4;
    //�Զ�����д�ķ�ʽ���ļ�
    fp=fopen(bmpName,"wb");
    if(fp==0) return 0;
    //����λͼ�ļ�ͷ�ṹ��������д�ļ�ͷ��Ϣ
    fileHead.bfType = 0x4D42;//bmp����
    //bfSize��ͼ���ļ�4����ɲ���֮��
    fileHead.bfSize= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)
                     + colorTablesize + lineByte*height;
    fileHead.bfReserved1 = 0;
    fileHead.bfReserved2 = 0;
    //bfOffBits��ͼ���ļ�ǰ3����������ռ�֮��
    fileHead.bfOffBits=54+colorTablesize;
    //д�ļ�ͷ���ļ�
    fwrite(&fileHead, sizeof(BITMAPFILEHEADER),1, fp);
    //����λͼ��Ϣͷ�ṹ��������д��Ϣͷ��Ϣ
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
    //дλͼ��Ϣͷ���ڴ�
    fwrite(&head, sizeof(BITMAPINFOHEADER),1, fp);
    //����Ҷ�ͼ������ɫ��д���ļ�
    if(biBitCount==8)
        fwrite(pColorTable, sizeof(RGBQUAD),256, fp);
    //дλͼ���ݽ��ļ�
    fwrite(imgBuf, height*lineByte, 1, fp);
    //�ر��ļ�
    fclose(fp);
    return 1;
}


/*****************************************
* �������ƣ�
*      average()
*����������
*    int isx-ͼ����ʼ������
*    int isy-ͼ����ʼ������
*    int ifx-ͼ����ֹ������
*    int ify-ͼ����ֹ������
*����ֵ��
*    int��iaveƽ��ֵ
*˵������һ�������ڵ�λͼ����ȡƽ��ֵ
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
* �������ƣ�
*      create_quadtree()
*����������
*    int isx-ͼ����ʼ������
*    int isy-ͼ����ʼ������
*    int ifx-ͼ����ֹ������
*    int ify-ͼ����ֹ������
*����ֵ��
*    pQtre��ͷָ��
*˵�������ÿռ��Ĳ���������һ��λͼ�ν����ķ֣��������Ĳ���,�������λ�ú���ɫ��Ϣ
***********************************************************************/
pQtre create_quadtree(int isx, int isy, int ifx, int ify, int lev)
{
    pQtre head;
    int i,j;
    int ave;
    int k=0;			//�Ƿ���Ҫ�����Ĳ���
    head=(pQtre)malloc(sizeof(QTree));
    head->il=0;		//��Ϊ��Ҷ�ӽ��
    lev++;			//	��������1
    ave = average(isx, isy, ifx, ify);//����������ڻҶ�ֵ�ľ�ֵ
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
    if(k==0)		//k == 0 �ݹ����
    {
        head->uclr = ave;//��ȡ�Ҷ�ֵ
        //��ȡ����λ��
        head->isx = isx;
        head->isy = isy;
        head->ifx = ifx;
        head->ify = ify;
        head->il = 1;//��ΪҶ�ӽڵ�;
        number++;
        head->ilev=lev;	//��ȡ������
        head->children[0] = NULL;
        head->children[1] = NULL;
        head->children[2] = NULL;
        head->children[3] = NULL;
    }
    else
    {
        head->children[0] = create_quadtree(isx, isy, (isx + ifx)/2, (isy + ify)/2, lev);//�ݹ齨���������� (����)
        head->children[1] = create_quadtree((isx + ifx)/2, isy, ifx, (isy + ify)/2, lev);//�ݹ齨���������� (����)
        head->children[2] = create_quadtree((isx + ifx)/2, (isy + ify)/2, ifx, ify, lev);//�ݹ齨���������� (����)
        head->children[3] = create_quadtree(isx, (isy + ify)/2, (isx + ifx)/2, ify, lev);//�ݹ齨���������������£�
    }
    return head;
}


/*****************************************
* �������ƣ�
*      visit_quad_tree()
*����������
*    pQtre head-�Ĳ�����ͷָ��
*����ֵ��
*    ��
*˵������������Ĳ�����Ҷ�ӽ�㣬��������Ϣ���뵽fbuffer��
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
* �������ƣ�
*      visit_quad_treem()
*����������
*    pQtre head-�Ĳ�����ͷָ��
*����ֵ��
*    ��
*˵������������Ĳ�����Ҷ�ӽ�㣬��������Ϣ���뵽fbufferm��
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
* �������ƣ�
*      save_quad_tree()
*����������
*    char *name-Ҫ���ͼ����Ϣ���ļ���
*	 pQtre head-��ȡ��ͼ����Ϣ���Ĳ���ͷ���
*����ֵ��
*    ��
*˵������������Ĳ�����Ҷ�ӽ�㣬�жϷֱ����Ƿ����256X256����������Ϣ���뵽��Ӧ�ڴ���
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
* �������ƣ�
*      unload()
*����������
*    char *name -�ļ�����
*����ֵ��
*    0Ϊʧ�ܣ�1Ϊ�ɹ�
*˵�������������ļ����֣���������ķֱ�����Ϣ��С�����ļ����н�ѹ�����ָ����ص�ĻҶ���Ϣ
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
        pColorTable = (RGBQUAD*)malloc(256*sizeof(RGBQUAD));//����洢�ռ�
    fread(pColorTable, sizeof(RGBQUAD),256, fil);
    //�������������ͼ��ÿ��������ռ���ֽ�����������4�ı�����
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
