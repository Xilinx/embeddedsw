/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/***************************** Include Files *********************************/
#include "xv_warp_init_l2.h"
#include "xv_warp_init_utils.h"
#include "sleep.h"
#include "xdebug.h"
#include <stdlib.h>

/************************** Constant Definitions *****************************/
#define PTR_OFFSET_SZ sizeof(u16)
typedef u16 offset_t;
#ifndef align_up
#define align_up(num, align) \
    (((num) + ((align) - 1)) & ~((align) - 1))
#endif

#define REMAP_FIX_ACC					4
#define REMAP_DESCRIPTOR_SIZE			256
#define REMAP_ADDR_WIDTH				32
#define XV_WAIT_FOR_FLUSH_DONE		    (25)
#define XV_WAIT_FOR_FLUSH_DONE_TIMEOUT	(2000)

/************************** Function Prototypes ******************************/
static void *XVWarpInit_aligned_malloc(size_t align, size_t size);
static void XVWarpInit_aligned_free(void * ptr);
static void XVWarpInit_OneTimeCalcs(XVWarpInitVector_Hw *initvector_hw, int *h);
static void XVWarpInit_OnetimeCalcsArbt(XVWarpInit_ArbParam *arbitrary_param,
					unsigned short fr_width, unsigned short fr_height);
static void XVWarpInit_SetDescriptor(XVWarpInitVector_Hw_Aligned *descptr,
		XVWarpInitVector_Hw *initvector_hw);
static void XVWarpInit_AllocArbMem(XVWarpInit_ArbParam *arbitrary_param,
		int grid_size, u16 fr_width, u16 fr_height);
static int XVWarpInit_ParseMeshInfo(XVWarpInit_ArbParam *arbitrary_param,
		XVWarpInit_ArbParam_MeshInfo *ctrl_pts,
		short fr_width, short fr_height);
static int XVWarpInit_ValidateInputConfigs(XV_warp_init *InstancePtr,
		XVWarpInit_InputConfigs *ConfigPtr);

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
* This function Enables the Interrupts for the core instance
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  Mask is the Interrupt enable mask
*
* @return none
*
******************************************************************************/
void XVWarpInit_EnableInterrupts(XV_warp_init *InstancePtr,
		u32 Mask)
{
	Xil_AssertVoid(InstancePtr);

	XV_warp_init_InterruptEnable(InstancePtr, Mask);
	XV_warp_init_InterruptGlobalEnable(InstancePtr);
}

/*****************************************************************************/
/**
* This function creates the descriptors.
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  num_desc is the number of descriptors to be created
*
* @return XST_SUCCESS if Descriptors created successfully
*         XST_FAILURE if Descriptors creation failed
*
******************************************************************************/
int XVWarpInit_SetNumOfDescriptors(XV_warp_init *InstancePtr,
		u32 num_desc)
{
	XVWarpInitVector_Hw_Aligned *descptr = NULL, *currptr, *prevptr;
	u32 descnum;

	Xil_AssertNonvoid(InstancePtr);

	for (descnum = 0; descnum < num_desc; descnum++)
	{
		currptr = XVWarpInit_aligned_malloc(InstancePtr->config->axi_mm_data_width/8,
				sizeof(XVWarpInitVector_Hw_Aligned));
		if (currptr == NULL)
			return XST_FAILURE;
		memset((u32 *)currptr, 0, sizeof(XVWarpInitVector_Hw_Aligned));

		if (descnum == 0)
			descptr = currptr;
		if (descnum > 0)
			prevptr->remap_nextaddr = (u64)currptr;
		if (descnum == (num_desc - 1))
			currptr->remap_nextaddr = (u64)0;

		prevptr = currptr;
	}

	InstancePtr->RemapVectorDesc_BaseAddr = (u64)descptr;
	InstancePtr->NumDescriptors = descnum;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function clears and de-allocates all the descriptors.
*
* @param  InstancePtr is a pointer to core instance to be worked upon
*
* @return none
*
******************************************************************************/
void XVWarpInit_ClearNumOfDescriptors(XV_warp_init *InstancePtr)
{
	XVWarpInitVector_Hw_Aligned *head, *tmpptr;

	Xil_AssertVoid(InstancePtr);

	head = (XVWarpInitVector_Hw_Aligned *)InstancePtr->RemapVectorDesc_BaseAddr;

	while(head->remap_nextaddr) {
		tmpptr = (XVWarpInitVector_Hw_Aligned *)head->remap_nextaddr;
		XVWarpInit_aligned_free(head);
		head = tmpptr;
	}

	XVWarpInit_aligned_free(head);
	InstancePtr->RemapVectorDesc_BaseAddr = 0;
	InstancePtr->NumDescriptors = 0;
}

/*****************************************************************************/
/**
* This function programs a descriptor with given configurations.
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  Descnum is the descriptor number which to be configured
* @param  ConfigPtr is the input configuration pointer which to be configured
* 					into the descriptor
*
* @return XST_SUCCESS if programming descriptor is successful
*         XST_FAILURE if input configurations are not valid.
*
******************************************************************************/
int XVWarpInit_ProgramDescriptor(XV_warp_init *InstancePtr,
		u32 Descnum, XVWarpInit_InputConfigs *ConfigPtr)
{
	XVWarpInitVector_Hw desc;
	XVWarpInit_ArbParam arbit_param;
	XVWarpInitVector_Hw_Aligned *descptr;

	Xil_AssertNonvoid(InstancePtr);

	if (Descnum >= InstancePtr->NumDescriptors) {
		xil_printf("Wrong descriptor\n\r");
		return XST_FAILURE;
	}

	if (XVWarpInit_ValidateInputConfigs(InstancePtr, ConfigPtr) != XST_SUCCESS)
		return XST_FAILURE;

	descptr = (XVWarpInitVector_Hw_Aligned *)InstancePtr->RemapVectorDesc_BaseAddr;
	for (u32 i = 0; i < Descnum; i++) {
		descptr = (XVWarpInitVector_Hw_Aligned *)descptr->remap_nextaddr;
	}

	desc.width	= ConfigPtr->width;
	desc.height	= ConfigPtr->height;
	desc.bytes_per_pixel = ConfigPtr->bytes_per_pixel;
	desc.warp_type = ConfigPtr->warp_type;
	desc.filter_table_addr_0 = ConfigPtr->filter_table_addr_0;
	desc.filter_table_addr_1 = ConfigPtr->filter_table_addr_1;
	desc.width_Q4 = desc.width << REMAP_FIX_ACC;
	desc.height_Q4 = desc.height << REMAP_FIX_ACC;

	if (desc.warp_type == DISTORTION_ARBITARY) {
		XVWarpInit_AllocArbMem(&arbit_param, ConfigPtr->num_ctrl_pts,
				desc.width, desc.height);
		XVWarpInit_ParseMeshInfo(&arbit_param, ConfigPtr->ctr_pts,
				desc.width, desc.height);
		desc.src_ctrl_x_pts	= ((u64)arbit_param.src_ctrl_x_pts)/4;
		desc.src_ctrl_y_pts	= ((u64)arbit_param.src_ctrl_y_pts)/4;
		desc.src_tangents_x	= ((u64)arbit_param.src_tangents_x)/4;
		desc.src_tangents_y	= ((u64)arbit_param.src_tangents_y)/4;
		desc.interm_x			= ((u64)arbit_param.interm_x)/4;
		desc.interm_y 		= ((u64)arbit_param.interm_y)/4;
		desc.num_ctrl_pts 	= ConfigPtr->num_ctrl_pts;

		XVWarpInit_OnetimeCalcsArbt(&arbit_param,
				desc.width, desc.height);
	} else {
		desc.k_pre	= ConfigPtr->k_pre;
		desc.k_post	= ConfigPtr->k_post;
		XVWarpInit_OneTimeCalcs(&desc, ConfigPtr->h);
	}

	XVWarpInit_SetDescriptor(descptr, &desc);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function starts the IP core with a selected descriptor configuration.
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  Descnum is the descriptor number with which configurations the IP
* 					shall start
*
* @return XST_SUCCESS if IP core start successful
*         XST_FAILURE if selected descriptor number is not valid.
*
******************************************************************************/
int XVWarpInit_start_with_desc(XV_warp_init *InstancePtr,
		u32 descnum)
{
	u64 remapvectoroffset;
	XVWarpInitVector_Hw_Aligned *descptr;

	Xil_AssertNonvoid(InstancePtr);

	descptr = (XVWarpInitVector_Hw_Aligned *)InstancePtr->RemapVectorDesc_BaseAddr;

	if (descnum >= InstancePtr->NumDescriptors) {
		xil_printf("Wrong descriptor\n\r");
		return XST_FAILURE;
	}

	for (u32 i = 0; i < descnum; i++) {
		descptr = (XVWarpInitVector_Hw_Aligned *)descptr->remap_nextaddr;
	}

	remapvectoroffset = ((u64)descptr)/4;

	XV_warp_init_Set_maxi_read_write(InstancePtr, 0);
	XV_warp_init_Set_desc_addr(InstancePtr, remapvectoroffset);

	XV_warp_init_Start(InstancePtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function stops the core instance
*
* @param  InstancePtr is a pointer to core instance to be worked upon
*
* @return none
*
******************************************************************************/
void XVWarpInit_Stop(XV_warp_init *InstancePtr)
{
  u32 Data = 0;
  u32 cnt = 0;

  Xil_AssertVoid(InstancePtr);

  /* Flush the core bit */
  XV_warp_init_SetFlushbit(InstancePtr);

  do {
    Data = XV_warp_init_Get_FlushDone(InstancePtr);
    usleep(XV_WAIT_FOR_FLUSH_DONE_TIMEOUT);
    cnt++;
  } while ((Data == 0) && (cnt < XV_WAIT_FOR_FLUSH_DONE));

  if (Data == 0)
        return;
}


/*****************************************************************************/
/*****************************************************************************/
/**
* This function creates alligned void memory pointer.
*
* @param	align is the alignment size.
* @param	size, it is the size of pointer to be created.
*
* @return	ptr is memory pointer to be returned.
*
******************************************************************************/
static void * XVWarpInit_aligned_malloc(size_t align, size_t size)
{
    void * ptr = NULL;

    /*
     * We want it to be a power of two since
     * align_up operates on powers of two
     */

    if(align && size)
    {
        /*
         * We know we have to fit an offset value
         * We also allocate extra bytes to ensure we
         * can meet the alignment
         */
        uint32_t hdr_size = PTR_OFFSET_SZ + (align - 1);
        void * p = malloc(size + hdr_size);

        if(p)
        {
            /*
             * Add the offset size to malloc's pointer
             * (we will always store that)
             * Then align the resulting value to the
             * target alignment
             */
            ptr = (void *) align_up(((uintptr_t)p + PTR_OFFSET_SZ), align);

            /*
             * Calculate the offset and store it
             * behind our aligned pointer
             */
            *((u16 *)ptr - 1) =
                (u16)((uintptr_t)ptr - (uintptr_t)p);

        } 	/* else NULL, could not malloc */
    } /* else NULL, invalid arguments */

    return ptr;
}

/*****************************************************************************/
/**
* This function frees the allocated aligned memory.
*
* @param	ptr is the memory pointer to be free.
*
* @return	None
*
******************************************************************************/
static void XVWarpInit_aligned_free(void * ptr)
{
    /*
    * Walk backwards from the passed-in pointer
    * to get the pointer offset. We convert to an offset_t
    * pointer and rely on pointer math to get the data
    */
    offset_t offset = *((offset_t *)ptr - 1);

    /*
    * Once we have the offset, we can get our
    * original pointer and call free
    */
    void * p = (void *)((uint8_t *)ptr - offset);
    free(p);
}

/*****************************************************************************/
/**
* This function sets the descriptor into hw.
*
* @param	descptr is pointer to hardware memory(DDR).
* @param	initvector_hw is the descriptor data to be set in DDR.
*
* @return	None
*
******************************************************************************/
static void XVWarpInit_SetDescriptor(XVWarpInitVector_Hw_Aligned *descptr,
		XVWarpInitVector_Hw *initvector_hw)
{
	descptr->width = initvector_hw->width;
	descptr->height = initvector_hw->height;
	descptr->k_pre = initvector_hw->k_pre;
	descptr->k_post = initvector_hw->k_post;
	descptr->k0_pre = initvector_hw->k0_pre;
	descptr->k1_pre = initvector_hw->k1_pre;
	descptr->k0_post = initvector_hw->k0_post;
	descptr->k1_post = initvector_hw->k1_post;
	descptr->Q_fact_pre = initvector_hw->Q_fact_pre;
	descptr->Q_fact_post = initvector_hw->Q_fact_post;
	descptr->k0_pre_Q_bits = initvector_hw->k0_pre_Q_bits;
	descptr->k0_post_Q_bits = initvector_hw->k0_post_Q_bits;
	descptr->k1_pre_Q_bits = initvector_hw->k1_pre_Q_bits;
	descptr->k1_post_Q_bits = initvector_hw->k1_post_Q_bits;
	descptr->normfactor = initvector_hw->normfactor;
	descptr->cenX = initvector_hw->cenX;
	descptr->cenY = initvector_hw->cenY;
	descptr->width_Q4 = initvector_hw->width_Q4;
	descptr->height_Q4 = initvector_hw->height_Q4;
	for (int i=0; i < 6; i++) {
		descptr->h[i] = initvector_hw->h[i];
		descptr->h_Qbits[i] = initvector_hw->h_Qbits[i];
	}
	descptr->h_trans[0] = initvector_hw->h_trans[0];
	descptr->h_trans[1] = initvector_hw->h_trans[1];
	descptr->h_trans[2] = initvector_hw->h_trans[2];

	descptr->filter_table_addr_0 = initvector_hw->filter_table_addr_0;
	descptr->filter_table_addr_1 = initvector_hw->filter_table_addr_1;
	descptr->src_ctrl_x_pts = initvector_hw->src_ctrl_x_pts;
	descptr->src_ctrl_y_pts = initvector_hw->src_ctrl_y_pts;
	descptr->src_tangents_x = initvector_hw->src_tangents_x;
	descptr->src_tangents_y = initvector_hw->src_tangents_y;
	descptr->interm_x = initvector_hw->interm_x;
	descptr->interm_y = initvector_hw->interm_y;

	descptr->num_ctrl_pts = initvector_hw->num_ctrl_pts;
	descptr->bytes_per_pixel = initvector_hw->bytes_per_pixel;
	descptr->warp_type = initvector_hw->warp_type;

	u32 *ptr = (u32 *)descptr;
	u32 checksum = 0;
	for (int i=0; i<=52; i++) {
		checksum ^= ptr[i];
	}
	descptr->driver_checksum = checksum;
}

/*****************************************************************************/
/**
* This function sets fixed point format for 3x3 cofficients of projection transform.
*
* @param	proj_trans, projective transform matrix of size 3x3.
*
* @return	h, fixed point representation of 3x2 of coefficents of proj_trans
*           i.e first two columns
* @return	h_trans, fixed point representation of 3x1 of coefficents of proj_trans
*           i.e last column
* @return	h_Qbits, fixed point type of each coefficient of proj_trans i.e 3x3
*
******************************************************************************/
static void estimate_projtrans_Qbits(int *proj_trans, short *h, int *h_trans,
		unsigned char *h_Qbits) {
	int sign_bit, i, j;
	unsigned char msb_bit;
	unsigned int ui_temp;

	j = 0;
	for (i = 0; i < 6; i++) {
		if ((i + 1) % 3) {
			sign_bit = proj_trans[i] >> 31;
			ui_temp = (proj_trans[i] ^ (sign_bit)) - sign_bit; //absolute value

			msb_bit = XVWarpInit_DominantBit(ui_temp);
			if (msb_bit >= 31)
				return;
			//msb_bit never should be 32
			//one bit for sign
			if (msb_bit > 15) {
				ui_temp >>= (msb_bit - 15);
				h_Qbits[j] = 30 - msb_bit;//24 - (msb_bit - 15) - 8
				if (sign_bit)
					h[j] = -1 * ui_temp;
				else
					h[j] = ui_temp;
			}
			else {
				h[j] = proj_trans[i];
				h_Qbits[j] = 15;
			}
			j++;
		}
	}

	for (i = 6; i < 9; i++) {
		if ((i + 1) % 3) {
			sign_bit = proj_trans[i] >> 31;
			ui_temp = (proj_trans[i] ^ (sign_bit)) - sign_bit; //absolute value

			msb_bit = XVWarpInit_DominantBit(ui_temp);
			//msb_bit should be less than 28
			if (msb_bit >= 27)
				return;
			//one bit for sign
			if (msb_bit > 15) {
				ui_temp >>= (msb_bit - 15);
				h_Qbits[j] = 26 - msb_bit;//24 - (msb_bit - 15) - 8
				if (sign_bit)
					h[j] = -1 * ui_temp;
				else
					h[j] = ui_temp;
			}
			else {
				h[j] = proj_trans[i];
				h_Qbits[j] = 11;
			}
			j++;
		}
	}

	h_trans[0] = proj_trans[2];
	h_trans[1] = proj_trans[5];
	h_trans[2] = proj_trans[8];
}

/*****************************************************************************/
/**
* This function does all the ontime calculation for lens distortion which to
* be done for each input configs change.
*
* @param	initvector_hw is the pointer to initialization vector.
* @param	proj_trans is the pointer to projective transformation input.
*
* @return	None
*
******************************************************************************/
static void XVWarpInit_OneTimeCalcs(XVWarpInitVector_Hw *initvector_hw,
		int *proj_trans)
{
	unsigned char int_bits;
	unsigned int tempx, tempy;
	unsigned int Q22_one, Q31_one;
	int cenX = initvector_hw->width >> 1;
	int cenY = initvector_hw->height >> 1;
	unsigned int Radius2 = (cenX * cenX) + (cenY * cenY);

	initvector_hw->normfactor = (unsigned int)(4294967295 / Radius2);

	initvector_hw->cenX = cenX << 4;;
	initvector_hw->cenY = cenY << 4;

	Q22_one = ((unsigned int)1) << 22;
	Q31_one = ((unsigned int)1) << 31;

	estimate_projtrans_Qbits(proj_trans, initvector_hw->h,
			initvector_hw->h_trans, initvector_hw->h_Qbits);

	initvector_hw->k1_pre = 0;
	initvector_hw->k0_pre = 0;
	initvector_hw->Q_fact_pre = 0;
	initvector_hw->k0_pre_Q_bits = 0;
	initvector_hw->k1_pre_Q_bits = 0;
	if (initvector_hw->k_pre)
	{
		if (initvector_hw->k_pre < 0) {
			initvector_hw->k1_pre = (unsigned short)(-initvector_hw->k_pre);
			tempx = XVWarpInit_ExponentialPos(initvector_hw->k1_pre);
			tempy = tempx - Q22_one;

			initvector_hw->Q_fact_pre = Q22_one;
		}
		else {
			initvector_hw->k1_pre = initvector_hw->k_pre;

			tempx = XVWarpInit_ExponentialNeg(initvector_hw->k1_pre);
			tempx >>= 9;
			tempy = Q22_one - tempx;

			initvector_hw->Q_fact_pre = Q31_one;
		}

		int_bits = XVWarpInit_DominantBit(initvector_hw->k1_pre >> 12);
		initvector_hw->k1_pre >>= int_bits;
		initvector_hw->k1_pre_Q_bits = 4 - int_bits;

		int_bits = XVWarpInit_DominantBit(tempy >> 22);

		tempx = (unsigned short)((tempy) >> (int_bits + 6));

		initvector_hw->k0_pre = XVWarpInit_Inverse(tempx, int_bits,
				&initvector_hw->k0_pre_Q_bits);

		initvector_hw->k0_pre >>= 2;
		initvector_hw->k0_pre *= 3;
		initvector_hw->k0_pre_Q_bits -= 2;
	}

	initvector_hw->k1_post = 0;
	initvector_hw->k0_post = 0;
	initvector_hw->k0_post_Q_bits = 0;
	initvector_hw->Q_fact_post = 0;
	initvector_hw->k1_post_Q_bits = 0;
	if (initvector_hw->k_post)
	{
		if (initvector_hw->k_post < 0) {
			initvector_hw->k1_post = (unsigned short)(-initvector_hw->k_post);
			tempx = XVWarpInit_ExponentialPos(initvector_hw->k1_post);
			tempy = tempx - Q22_one;

			initvector_hw->Q_fact_post = Q22_one;
		}
		else {
			initvector_hw->k1_post = initvector_hw->k_post;

			tempx = XVWarpInit_ExponentialNeg(initvector_hw->k1_post);
			tempx >>= 9;
			tempy = Q22_one - tempx;

			initvector_hw->Q_fact_post = Q31_one;
		}

		int_bits = XVWarpInit_DominantBit(initvector_hw->k1_post >> 12);
		initvector_hw->k1_post >>= int_bits;
		initvector_hw->k1_post_Q_bits = 4 - int_bits;

		int_bits = XVWarpInit_DominantBit(tempy >> 22);

		tempx = (unsigned short)((tempy) >> (int_bits + 6));

		initvector_hw->k0_post = XVWarpInit_Inverse(tempx, int_bits,
				&initvector_hw->k0_post_Q_bits);

		initvector_hw->k0_post >>= 2;
		initvector_hw->k0_post *= 3;
		initvector_hw->k0_post_Q_bits -= 2;
	}
}

/*****************************************************************************/
/**
* This function calculates remap vectors of a row vector.
*
* @param	knots_x, Grid control points in x direction.
* @param	knots_y, Grid control points in y direction.
* @param	grid_pts, Number of gird control points.
* @param	len, length of row vector.
*
* @return	remap_row, remap vectors of the row vector
*
******************************************************************************/
static void apply_arbt_warp_line(short *knots_x, short *knots_y,
		int grid_pts, int len, int *remap_row)
{
	int i, j, j1, j2;
	short x;
	int p1 = 0, p2, p3;
	int a0 = 0, a1 = 0, a2 = 0, a3 = 0;
	unsigned short diff;
	char n_bits, diff_bits, x0_bits;
	unsigned char integerbits;
	int dx0, dx1, dx2, t, x2;
	int dy0, dy1, dy2;
	long long ll_tmp;
	int a1x1, a2x2, a3x3;

	p3 = -1;
	j = 1;
	for (i = 0; i < len; i++) {
		p2 = i;
		if ((p2 > p3) && (j <= grid_pts)) {
			p1 = knots_x[j];
			p3 = knots_x[j + 1];

			j1 = j - 1;
			j2 = j + 2;

			diff = (unsigned short)(p3 - p1);
			integerbits = XVWarpInit_DominantBit(diff);
			diff_bits = 16 - integerbits;
			diff <<= diff_bits;
			dx0 = XVWarpInit_Inverse(diff, integerbits, &x0_bits);
			dy0 = knots_y[j + 1] - knots_y[j];
			dy0 *= dx0;
			dy0 >>= (x0_bits - 16);

			diff = (unsigned short)(p3 - knots_x[j1]);
			integerbits = XVWarpInit_DominantBit(diff);
			diff_bits = 16 - integerbits;
			diff <<= diff_bits;
			dx1 = XVWarpInit_Inverse(diff, integerbits, &n_bits);
			dy1 = knots_y[j + 1] - knots_y[j1];
			dy1 *= dx1;
			dy1 >>= (n_bits - 16);

			diff = (unsigned short)(knots_x[j2] - p1);
			integerbits = XVWarpInit_DominantBit(diff);
			diff_bits = 16 - integerbits;
			diff <<= diff_bits;
			dx2 = XVWarpInit_Inverse(diff, integerbits, &n_bits);
			dy2 = knots_y[j2] - knots_y[j];
			dy2 *= dx2;
			dy2 >>= (n_bits - 16);

			a0 = ((int)knots_y[j]) * 65536;

			a1 = dy1;

			a2 = (3 * dy0 - 2 * dy1 - dy2) >> 4;
			a2 *= dx0;
			a2 >>= (x0_bits - 12);

			t = (-2 * dy0 + dy1 + dy2) >> 4;
			ll_tmp = dx0;
			ll_tmp *= dx0;
			a3 = (ll_tmp * t) >> (2 * x0_bits - 24);

			j++;
		}

		x = p2 - p1;
		a1x1 = a1 * x;
		x2 = x * x;
		ll_tmp = a2;
		a2x2 = (ll_tmp * x2) >> 8;
		ll_tmp = a3;
		ll_tmp *= x;
		a3x3 = (ll_tmp * x2) >> 20;

		remap_row[i] = (a3x3 + a2x2 + a1x1 + a0) >> 12;
	}
}

/*****************************************************************************/
/**
* This function calculates tangent vectors source control points
*
* @param	src_ctrl_pts, dispaced control points.
* @param	fr_size, frame size.
* @param	grid_size, Number of gird control points.
*
* @return	src_tangents, tangent points of source input
*
******************************************************************************/
static void creat_src_tangents(unsigned short *src_ctrl_pts, int *src_tangents,
							short fr_size, short grid_size) {
	int *src_ptr;
	int l, i, seg_size;
	int dist_x1, dist_x2;
	char n_bits, diff_bits;
	unsigned char integerbits;

	src_ptr = src_tangents;
	if (!(fr_size%grid_size)) {
		seg_size = fr_size / grid_size;
		l = 0;
		for (i = 0; i <= fr_size; i += seg_size) {
			src_ctrl_pts[l++] = i;
		}

		integerbits = XVWarpInit_DominantBit(seg_size);
		diff_bits = 16 - integerbits;
		seg_size <<= diff_bits;
		dist_x1 = XVWarpInit_Inverse(seg_size, integerbits, &n_bits);
		//dist_x1 >>= (n_bits - 16);
		dist_x2 = dist_x1 >> 1;
		dist_x1 <<= 8;
		dist_x1 |= n_bits;
		dist_x2 <<= 8;
		dist_x2 |= n_bits;

		*src_ptr++ = dist_x1;
		*src_ptr++ = dist_x1;
		*src_ptr++ = dist_x2;
		for (i = 1; i < grid_size - 1; i++) {
			*src_ptr++ = dist_x1;
			*src_ptr++ = dist_x2;
			*src_ptr++ = dist_x2;
		}
		*src_ptr++ = dist_x1;
		*src_ptr++ = dist_x2;
		*src_ptr++ = dist_x1;
	}
	else {
		dist_x1 = fr_size;
		dist_x2 = (dist_x1 << 4) / grid_size;

		for (i = 0; i <= grid_size; i++) {
			src_ctrl_pts[i] = (short)(((i * dist_x2) + 8) >> 4);
		}

		seg_size = src_ctrl_pts[1] - src_ctrl_pts[0];
		integerbits = XVWarpInit_DominantBit(seg_size);
		diff_bits = 16 - integerbits;
		seg_size <<= diff_bits;
		dist_x1 = XVWarpInit_Inverse(seg_size, integerbits, &n_bits);
		dist_x1 <<= 8;
		dist_x1 |= n_bits;
		*src_ptr++ = dist_x1;

		*src_ptr++ = dist_x1;

		seg_size = src_ctrl_pts[2] - src_ctrl_pts[0];
		integerbits = XVWarpInit_DominantBit(seg_size);
		diff_bits = 16 - integerbits;
		seg_size <<= diff_bits;
		dist_x1 = XVWarpInit_Inverse(seg_size, integerbits, &n_bits);
		dist_x1 <<= 8;
		dist_x1 |= n_bits;
		*src_ptr++ = dist_x1;

		for (i = 1; i < (grid_size - 1); i++) {
			seg_size = src_ctrl_pts[i + 1] - src_ctrl_pts[i];
			integerbits = XVWarpInit_DominantBit(seg_size);
			diff_bits = 16 - integerbits;
			seg_size <<= diff_bits;
			dist_x1 = XVWarpInit_Inverse(seg_size, integerbits, &n_bits);
			dist_x1 <<= 8;
			dist_x1 |= n_bits;
			*src_ptr++ = dist_x1;

			seg_size = src_ctrl_pts[i + 1] - src_ctrl_pts[i - 1];
			integerbits = XVWarpInit_DominantBit(seg_size);
			diff_bits = 16 - integerbits;
			seg_size <<= diff_bits;
			dist_x1 = XVWarpInit_Inverse(seg_size, integerbits, &n_bits);
			dist_x1 <<= 8;
			dist_x1 |= n_bits;
			*src_ptr++ = dist_x1;

			seg_size = src_ctrl_pts[i + 2] - src_ctrl_pts[i];
			integerbits = XVWarpInit_DominantBit(seg_size);
			diff_bits = 16 - integerbits;
			seg_size <<= diff_bits;
			dist_x1 = XVWarpInit_Inverse(seg_size, integerbits, &n_bits);
			dist_x1 <<= 8;
			dist_x1 |= n_bits;
			*src_ptr++ = dist_x1;
		}

		seg_size = src_ctrl_pts[grid_size] - src_ctrl_pts[grid_size - 1];
		integerbits = XVWarpInit_DominantBit(seg_size);
		diff_bits = 16 - integerbits;
		seg_size <<= diff_bits;
		dist_x1 = XVWarpInit_Inverse(seg_size, integerbits, &n_bits);
		dist_x1 <<= 8;
		dist_x1 |= n_bits;
		*src_ptr++ = dist_x1;

		seg_size = src_ctrl_pts[grid_size] - src_ctrl_pts[grid_size - 2];
		integerbits = XVWarpInit_DominantBit(seg_size);
		diff_bits = 16 - integerbits;
		seg_size <<= diff_bits;
		dist_x1 = XVWarpInit_Inverse(seg_size, integerbits, &n_bits);
		dist_x1 <<= 8;
		dist_x1 |= n_bits;
		*src_ptr++ = dist_x1;

		*src_ptr++ = *(src_ptr - 2);
	}
}

/*****************************************************************************/
/**
* This function does all the ontime calculation for Arbitary distortion which
* to be done for each input configs change.
*
* @param	arbitrary_param is the pointer to input Arbitary parameters.
* @param	fr_width is the frame width.
* @param	fr_height is the frame height.
*
* @return	None
*
******************************************************************************/
static void XVWarpInit_OnetimeCalcsArbt(XVWarpInit_ArbParam *arbitrary_param,
	unsigned short fr_width, unsigned short fr_height) {
	u32 i, j, l;
	short *knots_x, *knots_y;
	int *t_data, *int_ptr;
	u32 grid_size;
	unsigned short *sh_ptr_x, *sh_ptr_y, num_pts;

	grid_size = arbitrary_param->grid_size;

	num_pts = grid_size + 1;
	knots_x = arbitrary_param->knots_x;
	knots_y = arbitrary_param->knots_y;

	//Col wise applying splines
	for (j = 0; j <= grid_size; j++) {
		sh_ptr_x = arbitrary_param->dst_ctrl_x_pts + j;
		sh_ptr_y = arbitrary_param->dst_ctrl_y_pts + j;
		l = 1;
		for (i = 0; i < arbitrary_param->num_ctrl_pts; i += num_pts) {
			knots_x[l] = sh_ptr_x[i];
			knots_y[l++] = sh_ptr_y[i];
		}

		knots_x[0] = knots_x[1];
		knots_y[0] = knots_y[1];
		knots_x[l] = knots_x[l - 1];
		knots_y[l] = knots_y[l - 1];

		apply_arbt_warp_line(knots_y, knots_x, grid_size,
				fr_height, arbitrary_param->temp_row);

		t_data = arbitrary_param->interm_y + j;
		int_ptr = arbitrary_param->temp_row;
		for (i = 0; i < fr_height; i++) {
			*t_data = *int_ptr;
			t_data += num_pts;
			int_ptr++;
		}
	}

	//Row wise applying splines
	for (j = 0; j <= grid_size; j++) {
		sh_ptr_x = arbitrary_param->dst_ctrl_x_pts + j * num_pts;
		sh_ptr_y = arbitrary_param->dst_ctrl_y_pts + j * num_pts;
		l = 1;
		for (i = 0; i < num_pts; i++) {
			knots_x[l] = sh_ptr_x[i];
			knots_y[l++] = sh_ptr_y[i];
		}

		knots_x[0] = knots_x[1];
		knots_y[0] = knots_y[1];
		knots_x[l] = knots_x[l - 1];
		knots_y[l] = knots_y[l - 1];

		apply_arbt_warp_line(knots_x, knots_y, grid_size,
				fr_width, arbitrary_param->temp_row);

		t_data = arbitrary_param->interm_x + j;
		int_ptr = arbitrary_param->temp_row;
		for (i = 0; i < fr_width; i++) {
			*t_data = *int_ptr;
			t_data += num_pts;
			int_ptr++;
		}
	}

	creat_src_tangents(arbitrary_param->src_ctrl_x_pts,
			arbitrary_param->src_tangents_x, fr_width, grid_size);

	creat_src_tangents(arbitrary_param->src_ctrl_y_pts,
			arbitrary_param->src_tangents_y, fr_height, grid_size);
}

/*****************************************************************************/
/**
* This function allocates the required memory for intermediate arbitary
* variables/calculation.
*
* @param	arbitrary_param is the pointer to input Arbitary parameters.
* @param	grid_size is the grid size for the arbitary distortion.
* @param	fr_width is the frame width.
* @param	fr_height is the frame height.
*
* @return	None
*
******************************************************************************/
static void XVWarpInit_AllocArbMem(XVWarpInit_ArbParam *arbitrary_param,
		int grid_size, u16 fr_width, u16 fr_height)
{
	int n_pts;
	int num_ctrl_pts;

	n_pts = grid_size + 1;
	num_ctrl_pts = n_pts * n_pts;

	arbitrary_param->grid_size = grid_size;
	arbitrary_param->num_ctrl_pts = num_ctrl_pts;

	arbitrary_param->dst_ctrl_x_pts = (unsigned short *)malloc(sizeof(unsigned short) * num_ctrl_pts);
	arbitrary_param->dst_ctrl_y_pts = (unsigned short *)malloc(sizeof(unsigned short) * num_ctrl_pts);
	arbitrary_param->src_ctrl_x_pts = (unsigned short *)malloc(sizeof(unsigned short) * n_pts);
	arbitrary_param->src_ctrl_y_pts = (unsigned short *)malloc(sizeof(unsigned short) * n_pts);

	arbitrary_param->src_tangents_x = (int *)malloc(sizeof(int) * grid_size * 3);
	arbitrary_param->src_tangents_y = (int *)malloc(sizeof(int) * grid_size * 3);
	arbitrary_param->knots_x = (short *)malloc(sizeof(short) * (n_pts+2));
	arbitrary_param->knots_y = (short *)malloc(sizeof(short) * (n_pts+2));
	arbitrary_param->interm_x = (int *)malloc(sizeof(int) * fr_width * n_pts);
	arbitrary_param->interm_y = (int *)malloc(sizeof(int) * fr_height * n_pts);
	arbitrary_param->temp_row = (int *)malloc(sizeof(int) * fr_width);
}

/*****************************************************************************/
/**
* This function parses the input mesinfo for arbitary distartion.
*
* @param	arbitrary_param is the pointer to input Arbitary parameters.
* @param	ctrl_pts is the pointer to the input mesh information.
* @param	fr_width is the frame width.
* @param	fr_height is the frame height.
*
* @return	XST_SUCCESS if the mesh information parsing successful
* 			XST_FAILURE if the control points are not valid in the given mesh
* 						information.
*
******************************************************************************/
static int XVWarpInit_ParseMeshInfo(XVWarpInit_ArbParam *arbitrary_param,
		XVWarpInit_ArbParam_MeshInfo *ctrl_pts,
		short fr_width, short fr_height)
{
	u32 n=arbitrary_param->grid_size, i;
	short seg_w, seg_h;
	int s_x, s_y, d_x, d_y;

	if (n < 2 || n > 32) {
		xil_printf("Wrong number of control points\n");
		return XST_FAILURE;
	}

	seg_w = fr_width / n;
	seg_h = fr_height / n;

	for (i = 0; i < arbitrary_param->num_ctrl_pts; i++) {
		s_x = ctrl_pts[i].s_x;
		s_y = ctrl_pts[i].s_y;
		d_x = ctrl_pts[i].d_x;
		d_y = ctrl_pts[i].d_y;
		if (abs(s_x - d_x) > seg_w || abs(s_y - d_y) > seg_h) {
			xil_printf("Maximum point movement should be half "
					"of distance between control points\n");
			return XST_FAILURE;
		}

		arbitrary_param->dst_ctrl_x_pts[i] = d_x;
		arbitrary_param->dst_ctrl_y_pts[i] = d_y;
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function validates the given configs with IP core configs.
*
* @param	InstancePtr is a pointer to core instance to be worked upon.
* @param	ConfigPtr is the pointer to input configuration
*
* @return	XST_SUCCESS if the validation is successful
* 			XST_FAILURE is any input config is invalid.
*
******************************************************************************/
static int XVWarpInit_ValidateInputConfigs(XV_warp_init *InstancePtr,
		XVWarpInit_InputConfigs *ConfigPtr)
{
	/*Check all the input configs with IP config parameters*/
	if (ConfigPtr->width > InstancePtr->config->max_width) {
		xdbg_printf(XDBG_DEBUG_ERROR,
				"Input width is not supported by the IP\n\r");
		return XST_FAILURE;
	}
	if (ConfigPtr->height > InstancePtr->config->max_height) {
		xdbg_printf(XDBG_DEBUG_ERROR,
				"Input height is not supported by the IP\n\r");
		return XST_FAILURE;
	}
	if (ConfigPtr->warp_type > InstancePtr->config->warp_type) {
		xdbg_printf(XDBG_DEBUG_ERROR,
				"warp type is not supported by the IP\n\r");
		return XST_FAILURE;
	}
	if (ConfigPtr->num_ctrl_pts > InstancePtr->config->max_control_pts) {
		xdbg_printf(XDBG_DEBUG_ERROR,
				"NUm of control points to be set is not supported by the IP\n\r");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
