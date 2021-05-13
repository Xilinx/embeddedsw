#ifndef INPUTS_H
#define INPUTS_H

#include "xwarp_input_configs.h"

/*
 * Mesh info for 1920x1080 with no arbitary distortion
 * control points - 8
 */
XVWarpInit_ArbParam_MeshInfo ctrl8_1080_nodist[] = {
	{0		,0		,64	    ,0   },
	{240	,0		,304	,0   },
	{480	,0		,444	,0   },
	{720	,0		,752	,0   },
	{960	,0		,1024	,0   },
	{1200	,0		,1264	,0   },
	{1440	,0		,1472	,0   },
	{1680	,0		,1712	,0   },
	{1920	,0		,1920	,0   },
	{0		,135	,0		,135 },
	{240	,135	,208	,135 },
	{480	,135	,448	,135 },
	{720	,135	,688	,135 },
	{960	,135	,960	,135 },
	{1200	,135	,1200	,135 },
	{1440	,135	,1376	,135 },
	{1680	,135	,1680	,135 },
	{1920	,135	,1920	,135 },
	{0		,270	,0		,270 },
	{240	,270	,240	,270 },
	{480	,270	,480	,270 },
	{720	,270	,720	,270 },
	{960	,270	,960	,270 },
	{1200	,270	,1200	,270 },
	{1440	,270	,1440	,270 },
	{1680	,270	,1680	,270 },
	{1920	,270	,1920	,270 },
	{0		,405	,0		,405 },
	{240	,405	,240	,405 },
	{480	,405	,480	,405 },
	{720	,405	,720	,405 },
	{960	,405	,960	,405 },
	{1200	,405	,1200	,405 },
	{1440	,405	,1440	,405 },
	{1680	,405	,1680	,405 },
	{1920	,405	,1920	,405 },
	{0		,540	,0		,540 },
	{240	,540	,240	,540 },
	{480	,540	,480	,540 },
	{720	,540	,720	,540 },
	{960	,540	,960	,540 },
	{1200	,540	,1200	,540 },
	{1440	,540	,1440	,540 },
	{1680	,540	,1680	,540 },
	{1920	,540	,1920	,540 },
	{0		,675	,0		,675 },
	{240	,675	,240	,675 },
	{480	,675	,480	,675 },
	{720	,675	,720	,675 },
	{960	,675	,960	,675 },
	{1200	,675	,1200	,675 },
	{1440	,675	,1440	,675 },
	{1680	,675	,1680	,675 },
	{1920	,675	,1920	,675 },
	{0		,810	,0		,810 },
	{240	,810	,240	,810 },
	{480	,810	,480	,810 },
	{720	,810	,720	,810 },
	{960	,810	,960	,810 },
	{1200	,810	,1200	,810 },
	{1440	,810	,1440	,810 },
	{1680	,810	,1680	,810 },
	{1920	,810	,1920	,810 },
	{0		,945	,0		,945 },
	{240	,945	,240	,945 },
	{480	,945	,480	,945 },
	{720	,945	,720	,945 },
	{960	,945	,960	,945 },
	{1200	,945	,1200	,945 },
	{1440	,945	,1440	,945 },
	{1680	,945	,1680	,945 },
	{1920	,945	,1920	,945 },
	{0		,1080	,32		,1080},
	{240	,1080	,272	,1080},
	{480	,1080	,448	,1080},
	{720	,1080	,752	,1080},
	{960	,1080	,896	,1080},
	{1200	,1080	,1264	,1080},
	{1440	,1080	,1408	,1080},
	{1680	,1080	,1680	,1080},
	{1920	,1080	,1920	,1080}
};

/*input configurations for warping*/
/*Lens distortion*/
WARP_CFG input_configs = {
	/* frame_width, frame_height, warp_type(0 -> L+H, 1 -> Arbitary) */
	   1920       , 1080        , 0        ,

	/* Lens parameters */
	/* pre_fisheye, post_fisheye */
	   0.0        , 0.0         ,

	/* Keystone parameters (Trapezoidal) */
	/* left_top_x, left_top_y, right_top_x, right_top_y, left_bottom_x, left_bottom_y, right_bottom_x, right_bottom_y */
       0         , 0         , 1920       , 0          , 0            , 1080         , 1920          , 1080          ,

	/* Keystone parameters (Affine) */
	/* scale_x, scale_y, rotation, trans_x, trans_y, zoom */
	   1.0    , 1.0    , 0.0     , 0.0    , 0.0    , 1.0 ,

	/* Arbitary parameters */
	/* num_ctrl_pts, ctrl_pts */
	   8           , ctrl8_1080_nodist
};


#if 0
/*Lens distortion*/
WARP_CFG input_configs = {
	/* frame_width, frame_height, warp_type(0 -> L+H, 1 -> Arbitary) */
	   1920       , 1080        , 0        ,

	/* Lens parameters */
	/* pre_fisheye, post_fisheye */
	   0.0        , 2.0         ,

	/* Keystone parameters (Trapezoidal) */
	/* left_top_x, left_top_y, right_top_x, right_top_y, left_bottom_x, left_bottom_y, right_bottom_x, right_bottom_y */
       0         , 0         , 1920       , 0          , 0            , 1080         , 1920          , 1080          ,

	/* Keystone parameters (Affine) */
	/* scale_x, scale_y, rotation, trans_x, trans_y, zoom */
	   1.0    , 1.0    , 0.0     , 0.0    , 0.0    , 1.0 ,

	/* Arbitary parameters */
	/* num_ctrl_pts, ctrl_pts */
	   8           , ctrl8_1080_nodist
};

/* Keystone distortion*/
WARP_CFG input_configs = {
	/* frame_width, frame_height, warp_type(0 -> L+H, 1 -> Arbitary) */
	   1920       , 1080        , 0        ,

	/* Lens parameters*/
	/* pre_fisheye, post_fisheye            */
	   0.0        , 0.0         ,

	/* Keystone parameters (Trapezoidal parameters)*/
	/* left_top_x, left_top_y, right_top_x, right_top_y, left_bottom_x, left_bottom_y, right_bottom_x, right_bottom_y */
       480       , 0         , 1440       , 0          , 0            , 1080         , 1920          , 1080          ,

	/* Keystone parameters (Affine)*/
	/* scale_x, scale_y, rotation, trans_x, trans_y, zoom */
	   1.0    , 1.0    , 0.0     , 0.0    , 0.0    , 1.0 ,

	/*Arbitary parameters*/
	/* num_ctrl_pts, ctrl_pts */
	   8           , ctrl8_1080_nodist
};

/*Lens + Keystone distortion*/
WARP_CFG input_configs = {
	/* frame_width, frame_height, warp_type(0 -> L+H, 1 -> Arbitary) */
	   1920       , 1080        , 0        ,

	/* Lens parameters*/
	/* pre_fisheye, post_fisheye            */
	   0.0        , 2.0         ,

	/* Keystone parameters (Trapezoidal)*/
	/* left_top_x, left_top_y, right_top_x, right_top_y, left_bottom_x, left_bottom_y, right_bottom_x, right_bottom_y */
       280         , 0         , 1640       , 0          , 0            , 1080         , 1920          , 1080          ,

	/* Keystone parameters (Affine)*/
	/* scale_x, scale_y, rotation, trans_x, trans_y, zoom */
	   1.0    , 1.0    , 0.0     , 0.0    , 0.0    , 1.0 ,

	/*Arbitary parameters*/
	/* num_ctrl_pts, ctrl_pts */
	   8           , ctrl8_1080_nodist
};

/*Arbitary distortion*/
WARP_CFG input_configs = {
	/* frame_width, frame_height, warp_type(0 -> L+H, 1 -> Arbitary) */
	   1920       , 1080        , 1        ,

	/* Lens parameters*/
	/* pre_fisheye, post_fisheye            */
	   0.0        , 0.0         ,

	/* Keystone parameters (Trapezoidal)*/
	/* left_top_x, left_top_y, right_top_x, right_top_y, left_bottom_x, left_bottom_y, right_bottom_x, right_bottom_y */
       0         , 0         , 1920       , 0          , 0            , 1080         , 1920          , 1080          ,

	/* Keystone parameters (Affine)*/
	/* scale_x, scale_y, rotation, trans_x, trans_y, zoom */
	   1.0    , 1.0    , 0.0     , 0.0    , 0.0    , 1.0 ,

	/* Arbitary parameters*/
	/* num_ctrl_pts, ctrl_pts */
	   8           , ctrl8_1080_nodist
};
#endif

#endif
