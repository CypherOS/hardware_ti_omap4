/*
 * Copyright (C) Texas Instruments - http://www.ti.com/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
* @file OMX3A.cpp
*
* This file contains functionality for handling 3A configurations.
*
*/

#undef LOG_TAG

#define LOG_TAG "CameraHAL"

#include "CameraHal.h"
#include "OMXCameraAdapter.h"
#include "ErrorUtils.h"

#undef TRUE
#undef FALSE
#define TRUE "true"
#define FALSE "false"

#define METERING_AREAS_RANGE 0xFF

namespace android {

status_t OMXCameraAdapter::setParameters3A(const CameraParameters &params,
                                           BaseCameraAdapter::AdapterState state)
{
    status_t ret = NO_ERROR;
    int mode = 0;
    const char *str = NULL;

    LOG_FUNCTION_NAME;

    str = params.get(TICameraParameters::KEY_EXPOSURE_MODE);
    mode = getLUTvalue_HALtoOMX( str, ExpLUT);
    if ( ( str != NULL ) && ( mParameters3A.Exposure != mode ))
        {
        mParameters3A.Exposure = mode;
        CAMHAL_LOGDB("Exposure mode %d", mode);
        if ( 0 <= mParameters3A.Exposure )
            {
            mPending3Asettings |= SetExpMode;
            }
        }

    str = params.get(CameraParameters::KEY_WHITE_BALANCE);
    mode = getLUTvalue_HALtoOMX( str, WBalLUT);
    if (mFirstTimeInit || ((str != NULL) && (mode != mParameters3A.WhiteBallance)))
        {
        mParameters3A.WhiteBallance = mode;
        CAMHAL_LOGDB("Whitebalance mode %d", mode);
        if ( 0 <= mParameters3A.WhiteBallance )
            {
            mPending3Asettings |= SetWhiteBallance;
            }
        }

    if ( 0 <= params.getInt(TICameraParameters::KEY_CONTRAST) )
        {
        if ( mFirstTimeInit ||
             ( (mParameters3A.Contrast  + CONTRAST_OFFSET) !=
                     params.getInt(TICameraParameters::KEY_CONTRAST)) )
            {
            mParameters3A.Contrast = params.getInt(TICameraParameters::KEY_CONTRAST) - CONTRAST_OFFSET;
            CAMHAL_LOGDB("Contrast %d", mParameters3A.Contrast);
            mPending3Asettings |= SetContrast;
            }
        }

    if ( 0 <= params.getInt(TICameraParameters::KEY_SHARPNESS) )
        {
        if ( mFirstTimeInit ||
             ((mParameters3A.Sharpness + SHARPNESS_OFFSET) !=
                     params.getInt(TICameraParameters::KEY_SHARPNESS)))
            {
            mParameters3A.Sharpness = params.getInt(TICameraParameters::KEY_SHARPNESS) - SHARPNESS_OFFSET;
            CAMHAL_LOGDB("Sharpness %d", mParameters3A.Sharpness);
            mPending3Asettings |= SetSharpness;
            }
        }

    if ( 0 <= params.getInt(TICameraParameters::KEY_SATURATION) )
        {
        if ( mFirstTimeInit ||
             ((mParameters3A.Saturation + SATURATION_OFFSET) !=
                     params.getInt(TICameraParameters::KEY_SATURATION)) )
            {
            mParameters3A.Saturation = params.getInt(TICameraParameters::KEY_SATURATION) - SATURATION_OFFSET;
            CAMHAL_LOGDB("Saturation %d", mParameters3A.Saturation);
            mPending3Asettings |= SetSaturation;
            }
        }

    if ( 0 <= params.getInt(TICameraParameters::KEY_BRIGHTNESS) )
        {
        if ( mFirstTimeInit ||
             (( mParameters3A.Brightness !=
                     ( unsigned int ) params.getInt(TICameraParameters::KEY_BRIGHTNESS))) )
            {
            mParameters3A.Brightness = (unsigned)params.getInt(TICameraParameters::KEY_BRIGHTNESS);
            CAMHAL_LOGDB("Brightness %d", mParameters3A.Brightness);
            mPending3Asettings |= SetBrightness;
            }
        }

    str = params.get(CameraParameters::KEY_ANTIBANDING);
    mode = getLUTvalue_HALtoOMX(str,FlickerLUT);
    if ( mFirstTimeInit || ( ( str != NULL ) && ( mParameters3A.Flicker != mode ) ))
        {
        mParameters3A.Flicker = mode;
        CAMHAL_LOGDB("Flicker %d", mParameters3A.Flicker);
        if ( 0 <= mParameters3A.Flicker )
            {
            mPending3Asettings |= SetFlicker;
            }
        }

    str = params.get(TICameraParameters::KEY_ISO);
    mode = getLUTvalue_HALtoOMX(str, IsoLUT);
    CAMHAL_LOGVB("ISO mode arrived in HAL : %s", str);
    if ( mFirstTimeInit || (  ( str != NULL ) && ( mParameters3A.ISO != mode )) )
        {
        mParameters3A.ISO = mode;
        CAMHAL_LOGDB("ISO %d", mParameters3A.ISO);
        if ( 0 <= mParameters3A.ISO )
            {
            mPending3Asettings |= SetISO;
            }
        }

    str = params.get(CameraParameters::KEY_FOCUS_MODE);
    mode = getLUTvalue_HALtoOMX(str, FocusLUT);
    if ( (mFirstTimeInit || ((str != NULL) && (mParameters3A.Focus != mode))))
        {
        //Apply focus mode immediatly only if  CAF  or Inifinity are selected
        if ( ( mode == OMX_IMAGE_FocusControlAuto ) ||
             ( mode == OMX_IMAGE_FocusControlAutoInfinity ) )
            {
            mPending3Asettings |= SetFocus;
            }

        mParameters3A.Focus = mode;
        CAMHAL_LOGDB("Focus %x", mParameters3A.Focus);
        }

    str = params.get(CameraParameters::KEY_EXPOSURE_COMPENSATION);
    if ( mFirstTimeInit ||
          (( str != NULL ) &&
                  (mParameters3A.EVCompensation !=
                          params.getInt(CameraParameters::KEY_EXPOSURE_COMPENSATION))))
        {
        CAMHAL_LOGDB("Setting EV Compensation to %d",
                     params.getInt(CameraParameters::KEY_EXPOSURE_COMPENSATION));

        mParameters3A.EVCompensation = params.getInt(CameraParameters::KEY_EXPOSURE_COMPENSATION);
        mPending3Asettings |= SetEVCompensation;
        }

    str = params.get(CameraParameters::KEY_SCENE_MODE);
    mode = getLUTvalue_HALtoOMX( str, SceneLUT);
    if (  mFirstTimeInit || (( str != NULL ) && ( mParameters3A.SceneMode != mode )) )
        {
        if ( 0 <= mode )
            {
            mParameters3A.SceneMode = mode;
            mPending3Asettings |= SetSceneMode;
            }
        else
            {
            mParameters3A.SceneMode = OMX_Manual;
            }

        CAMHAL_LOGDB("SceneMode %d", mParameters3A.SceneMode);
        }

    str = params.get(CameraParameters::KEY_FLASH_MODE);
    mode = getLUTvalue_HALtoOMX( str, FlashLUT);
    if (  mFirstTimeInit || (( str != NULL ) && ( mParameters3A.FlashMode != mode )) )
        {
        if ( 0 <= mode )
            {
            mParameters3A.FlashMode = mode;
            mPending3Asettings |= SetFlash;
            }
        else
            {
            mParameters3A.FlashMode = OMX_Manual;
            }
        }

    CAMHAL_LOGVB("Flash Setting %s", str);
    CAMHAL_LOGVB("FlashMode %d", mParameters3A.FlashMode);

    str = params.get(CameraParameters::KEY_EFFECT);
    mode = getLUTvalue_HALtoOMX( str, EffLUT);
    if (  mFirstTimeInit || (( str != NULL ) && ( mParameters3A.Effect != mode )) )
        {
        mParameters3A.Effect = mode;
        CAMHAL_LOGDB("Effect %d", mParameters3A.Effect);
        if ( 0 <= mParameters3A.Effect )
            {
            mPending3Asettings |= SetEffect;
            }
        }

    str = params.get(CameraParameters::KEY_AUTO_EXPOSURE_LOCK_SUPPORTED);
    if ( (str != NULL) && (!strcmp(str, "true")) )
      {
        OMX_BOOL lock = OMX_FALSE;
        mUserSetExpLock = OMX_FALSE;
        str = params.get(CameraParameters::KEY_AUTO_EXPOSURE_LOCK);
        if ( (strcmp(str, "true")) == 0)
          {
            CAMHAL_LOGVA("Locking Exposure");
            lock = OMX_TRUE;
            mUserSetExpLock = OMX_TRUE;
          }
        else
          {
            CAMHAL_LOGVA("UnLocking Exposure");
          }

        if (mParameters3A.ExposureLock != lock)
          {
            mParameters3A.ExposureLock = lock;
            CAMHAL_LOGDB("ExposureLock %d", lock);
            mPending3Asettings |= SetExpLock;
          }
      }

    str = params.get(CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK_SUPPORTED);
    if ( (str != NULL) && (!strcmp(str, "true")) )
      {
        OMX_BOOL lock = OMX_FALSE;
        mUserSetWbLock = OMX_FALSE;
        str = params.get(CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK);
        if ( (strcmp(str, "true")) == 0)
          {
            CAMHAL_LOGVA("Locking WhiteBalance");
            lock = OMX_TRUE;
            mUserSetWbLock = OMX_TRUE;
          }
        else
          {
            CAMHAL_LOGVA("UnLocking WhiteBalance");
          }
        if (mParameters3A.WhiteBalanceLock != lock)
          {
            mParameters3A.WhiteBalanceLock = lock;
            CAMHAL_LOGDB("WhiteBalanceLock %d", lock);
            mPending3Asettings |= SetWBLock;
          }
      }

    str = params.get(CameraParameters::KEY_METERING_AREAS);
    if ( (str != NULL) )
        {
        size_t MAX_METERING_AREAS;
        MAX_METERING_AREAS = atoi(params.get(CameraParameters::KEY_MAX_NUM_METERING_AREAS));

        mMeteringAreas.clear();

        CameraArea::parseFocusArea(str, strlen(str), mMeteringAreas);

        if ( MAX_METERING_AREAS > mMeteringAreas.size() )
            {
            CAMHAL_LOGDB("Setting Metering Areas %s",
                    params.get(CameraParameters::KEY_METERING_AREAS));

            mPending3Asettings |= SetMeteringAreas;
            }
        else
            {
            CAMHAL_LOGEB("Metering areas supported %d, metering areas set %d",
                         MAX_METERING_AREAS, mMeteringAreas.size());
            }
        }

    LOG_FUNCTION_NAME_EXIT;

    return ret;
}

int OMXCameraAdapter::getLUTvalue_HALtoOMX(const char * HalValue, LUTtype LUT)
{
    int LUTsize = LUT.size;
    if( HalValue )
        for(int i = 0; i < LUTsize; i++)
            if( 0 == strcmp(LUT.Table[i].userDefinition, HalValue) )
                return LUT.Table[i].omxDefinition;

    return -ENOENT;
}

const char* OMXCameraAdapter::getLUTvalue_OMXtoHAL(int OMXValue, LUTtype LUT)
{
    int LUTsize = LUT.size;
    for(int i = 0; i < LUTsize; i++)
        if( LUT.Table[i].omxDefinition == OMXValue )
            return LUT.Table[i].userDefinition;

    return NULL;
}

status_t OMXCameraAdapter::apply3ADefaults(Gen3A_settings &Gen3A)
{
    status_t ret = NO_ERROR;

    LOG_FUNCTION_NAME;

    Gen3A.Effect = getLUTvalue_HALtoOMX(OMXCameraAdapter::DEFAULT_EFFECT, EffLUT);
    ret |= setEffect(Gen3A);

    Gen3A.FlashMode = getLUTvalue_HALtoOMX(OMXCameraAdapter::DEFAULT_FLASH_MODE, FlashLUT);
    ret |= setFlashMode(Gen3A);

    Gen3A.SceneMode = getLUTvalue_HALtoOMX(OMXCameraAdapter::DEFAULT_SCENE_MODE, SceneLUT);
    ret |= setScene(Gen3A);

    Gen3A.EVCompensation = atoi(OMXCameraAdapter::DEFAULT_EV_COMPENSATION);
    ret |= setEVCompensation(Gen3A);

    Gen3A.Focus = getLUTvalue_HALtoOMX(OMXCameraAdapter::DEFAULT_FOCUS_MODE, FocusLUT);
    ret |= setFocusMode(Gen3A);

    Gen3A.ISO = getLUTvalue_HALtoOMX(OMXCameraAdapter::DEFAULT_ISO_MODE, IsoLUT);
    ret |= setISO(Gen3A);

    Gen3A.Flicker = getLUTvalue_HALtoOMX(OMXCameraAdapter::DEFAULT_ANTIBANDING, FlickerLUT);
    ret |= setFlicker(Gen3A);

    Gen3A.Brightness = atoi(OMXCameraAdapter::DEFAULT_BRIGHTNESS);
    ret |= setBrightness(Gen3A);

    Gen3A.Saturation = atoi(OMXCameraAdapter::DEFAULT_SATURATION);
    ret |= setSaturation(Gen3A);

    Gen3A.Sharpness = atoi(OMXCameraAdapter::DEFAULT_SHARPNESS);
    ret |= setSharpness(Gen3A);

    Gen3A.Contrast = atoi(OMXCameraAdapter::DEFAULT_CONTRAST);
    ret |= setContrast(Gen3A);

    Gen3A.WhiteBallance = getLUTvalue_HALtoOMX(OMXCameraAdapter::DEFAULT_WB, WBalLUT);
    ret |= setWBMode(Gen3A);

    Gen3A.Exposure = getLUTvalue_HALtoOMX(OMXCameraAdapter::DEFAULT_EXPOSURE_MODE, ExpLUT);
    ret |= setExposureMode(Gen3A);

    Gen3A.ExposureLock = OMX_FALSE;
    ret |= setExposureLock(Gen3A);

    Gen3A.FocusLock = OMX_FALSE;
    ret |= setFocusLock(Gen3A);

    Gen3A.WhiteBalanceLock = OMX_FALSE;
    ret |= setWhiteBalanceLock(Gen3A);

    LOG_FUNCTION_NAME_EXIT;

    return NO_ERROR;
}

status_t OMXCameraAdapter::setExposureMode(Gen3A_settings& Gen3A)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_CONFIG_EXPOSURECONTROLTYPE exp;

    LOG_FUNCTION_NAME;

    if ( OMX_StateInvalid == mComponentState )
        {
        CAMHAL_LOGEA("OMX component is in invalid state");
        return NO_INIT;
        }

    OMX_INIT_STRUCT_PTR (&exp, OMX_CONFIG_EXPOSURECONTROLTYPE);
    exp.nPortIndex = OMX_ALL;
    exp.eExposureControl = (OMX_EXPOSURECONTROLTYPE)Gen3A.Exposure;

///FIXME: Face priority exposure metering is not stable because of varying face sizes
///coming from the FD module. So disabling it for now.
#if 0
    if ( mFaceDetectionRunning )
        {
        //Disable Region priority and enable Face priority
        setAlgoPriority(REGION_PRIORITY, EXPOSURE_ALGO, false);
        setAlgoPriority(FACE_PRIORITY, EXPOSURE_ALGO, true);
        }
    else
#endif
        {
        //Disable Region priority and Face priority
        setAlgoPriority(REGION_PRIORITY, EXPOSURE_ALGO, false);
        setAlgoPriority(FACE_PRIORITY, EXPOSURE_ALGO, false);
        }

    eError =  OMX_SetConfig(mCameraAdapterParameters.mHandleComp,
                            OMX_IndexConfigCommonExposure,
                            &exp);
    if ( OMX_ErrorNone != eError )
        {
        CAMHAL_LOGEB("Error while configuring exposure mode 0x%x", eError);
        }
    else
        {
        CAMHAL_LOGDA("Camera exposure mode configured successfully");
        }

    LOG_FUNCTION_NAME_EXIT;

    return ErrorUtils::omxToAndroidError(eError);
}

status_t OMXCameraAdapter::setFlashMode(Gen3A_settings& Gen3A)
{
    status_t ret = NO_ERROR;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_IMAGE_PARAM_FLASHCONTROLTYPE flash;
    OMX_CONFIG_FOCUSASSISTTYPE focusAssist;

    LOG_FUNCTION_NAME;

    if ( OMX_StateInvalid == mComponentState )
        {
        CAMHAL_LOGEA("OMX component is in invalid state");
        return NO_INIT;
        }

    OMX_INIT_STRUCT_PTR (&flash, OMX_IMAGE_PARAM_FLASHCONTROLTYPE);
    flash.nPortIndex = OMX_ALL;
    flash.eFlashControl = ( OMX_IMAGE_FLASHCONTROLTYPE ) Gen3A.FlashMode;

    CAMHAL_LOGDB("Configuring flash mode 0x%x", flash.eFlashControl);
    eError =  OMX_SetConfig(mCameraAdapterParameters.mHandleComp,
                            (OMX_INDEXTYPE) OMX_IndexConfigFlashControl,
                            &flash);
    if ( OMX_ErrorNone != eError )
        {
        CAMHAL_LOGEB("Error while configuring flash mode 0x%x", eError);
        }
    else
        {
        CAMHAL_LOGDA("Camera flash mode configured successfully");
        }

    if ( OMX_ErrorNone == eError )
        {
        OMX_INIT_STRUCT_PTR (&focusAssist, OMX_CONFIG_FOCUSASSISTTYPE);
        focusAssist.nPortIndex = OMX_ALL;
        if ( flash.eFlashControl == OMX_IMAGE_FlashControlOff )
            {
            focusAssist.bFocusAssist = OMX_FALSE;
            }
        else
            {
            focusAssist.bFocusAssist = OMX_TRUE;
            }

        CAMHAL_LOGDB("Configuring AF Assist mode 0x%x", focusAssist.bFocusAssist);
        eError =  OMX_SetConfig(mCameraAdapterParameters.mHandleComp,
                                (OMX_INDEXTYPE) OMX_IndexConfigFocusAssist,
                                &focusAssist);
        if ( OMX_ErrorNone != eError )
            {
            CAMHAL_LOGEB("Error while configuring AF Assist mode 0x%x", eError);
            }
        else
            {
            CAMHAL_LOGDA("Camera AF Assist  mode configured successfully");
            }
        }

    LOG_FUNCTION_NAME_EXIT;

    return ErrorUtils::omxToAndroidError(eError);
}

status_t OMXCameraAdapter::setFocusMode(Gen3A_settings& Gen3A)
{
    status_t ret = NO_ERROR;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_IMAGE_CONFIG_FOCUSCONTROLTYPE focus;
    size_t top, left, width, height, weight;
    sp<CameraArea> focusArea = NULL;

    LOG_FUNCTION_NAME;

    BaseCameraAdapter::AdapterState state;
    BaseCameraAdapter::getState(state);

    if ( OMX_StateInvalid == mComponentState )
        {
        CAMHAL_LOGEA("OMX component is in invalid state");
        return NO_INIT;
        }

        {
        Mutex::Autolock lock(mFocusAreasLock);

        if ( !mFocusAreas.isEmpty() )
            {
            focusArea = mFocusAreas.itemAt(0);
            }
        }

    ///Face detection takes precedence over touch AF
    if ( mFaceDetectionRunning )
        {
        //Disable region priority first
        setAlgoPriority(REGION_PRIORITY, FOCUS_ALGO, false);

        //Enable face algorithm priority for focus
        setAlgoPriority(FACE_PRIORITY, FOCUS_ALGO , true);

        //Do normal focus afterwards
        ////FIXME: Check if the extended focus control is needed? this overrides caf
        //focusControl.eFocusControl = ( OMX_IMAGE_FOCUSCONTROLTYPE ) OMX_IMAGE_FocusControlExtended;
        }
    else if ( ( NULL != focusArea.get() ) && ( focusArea->isValid() ) )
        {

        //Disable face priority first
        setAlgoPriority(FACE_PRIORITY, FOCUS_ALGO, false);

        //Enable region algorithm priority
        setAlgoPriority(REGION_PRIORITY, FOCUS_ALGO, true);

        setTouchFocus();

        //Do normal focus afterwards
        //FIXME: Check if the extended focus control is needed? this overrides caf
        //focus.eFocusControl = ( OMX_IMAGE_FOCUSCONTROLTYPE ) OMX_IMAGE_FocusControlExtended;

        }
    else
        {

        //Disable both region and face priority
        setAlgoPriority(REGION_PRIORITY, FOCUS_ALGO, false);

        setAlgoPriority(FACE_PRIORITY, FOCUS_ALGO, false);

        }

    if ( NO_ERROR == ret && ((state & AF_ACTIVE) == 0) )
        {
        OMX_INIT_STRUCT_PTR (&focus, OMX_IMAGE_CONFIG_FOCUSCONTROLTYPE);
        focus.nPortIndex = mCameraAdapterParameters.mPrevPortIndex;
        focus.eFocusControl = (OMX_IMAGE_FOCUSCONTROLTYPE)Gen3A.Focus;

        CAMHAL_LOGDB("Configuring focus mode 0x%x", focus.eFocusControl);
        eError = OMX_SetConfig( mCameraAdapterParameters.mHandleComp, OMX_IndexConfigFocusControl, &focus);
        if ( OMX_ErrorNone != eError )
            {
            CAMHAL_LOGEB("Error while configuring focus mode 0x%x", eError);
            }
        else
            {
            CAMHAL_LOGDA("Camera focus mode configured successfully");
            }
        }

    LOG_FUNCTION_NAME_EXIT;

    return ErrorUtils::omxToAndroidError(eError);
}

status_t OMXCameraAdapter::setScene(Gen3A_settings& Gen3A)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_CONFIG_SCENEMODETYPE scene;

    LOG_FUNCTION_NAME;

    if ( OMX_StateInvalid == mComponentState )
        {
        CAMHAL_LOGEA("OMX component is in invalid state");
        return NO_INIT;
        }

    OMX_INIT_STRUCT_PTR (&scene, OMX_CONFIG_SCENEMODETYPE);
    scene.nPortIndex = OMX_ALL;
    scene.eSceneMode = ( OMX_SCENEMODETYPE ) Gen3A.SceneMode;

    CAMHAL_LOGDB("Configuring scene mode 0x%x", scene.eSceneMode);
    eError =  OMX_SetConfig(mCameraAdapterParameters.mHandleComp,
                            ( OMX_INDEXTYPE ) OMX_TI_IndexConfigSceneMode,
                            &scene);
    if ( OMX_ErrorNone != eError )
        {
        CAMHAL_LOGEB("Error while configuring scene mode 0x%x", eError);
        }
    else
        {
        CAMHAL_LOGDA("Camera scene configured successfully");
        }

    LOG_FUNCTION_NAME_EXIT;

    return ErrorUtils::omxToAndroidError(eError);
}

status_t OMXCameraAdapter::setEVCompensation(Gen3A_settings& Gen3A)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_CONFIG_EXPOSUREVALUETYPE expValues;

    LOG_FUNCTION_NAME;

    if ( OMX_StateInvalid == mComponentState )
        {
        CAMHAL_LOGEA("OMX component is in invalid state");
        return NO_INIT;
        }

    OMX_INIT_STRUCT_PTR (&expValues, OMX_CONFIG_EXPOSUREVALUETYPE);
    expValues.nPortIndex = mCameraAdapterParameters.mPrevPortIndex;

    OMX_GetConfig( mCameraAdapterParameters.mHandleComp,
                   OMX_IndexConfigCommonExposureValue,
                   &expValues);
    CAMHAL_LOGDB("old EV Compensation for OMX = 0x%x", (int)expValues.xEVCompensation);
    CAMHAL_LOGDB("EV Compensation for HAL = %d", Gen3A.EVCompensation);

    expValues.xEVCompensation = ( Gen3A.EVCompensation * ( 1 << Q16_OFFSET ) )  / 10;
    eError = OMX_SetConfig( mCameraAdapterParameters.mHandleComp,
                            OMX_IndexConfigCommonExposureValue,
                            &expValues);
    CAMHAL_LOGDB("new EV Compensation for OMX = 0x%x", (int)expValues.xEVCompensation);
    if ( OMX_ErrorNone != eError )
        {
        CAMHAL_LOGEB("Error while configuring EV Compensation 0x%x error = 0x%x",
                     ( unsigned int ) expValues.xEVCompensation,
                     eError);
        }
    else
        {
        CAMHAL_LOGDB("EV Compensation 0x%x configured successfully",
                     ( unsigned int ) expValues.xEVCompensation);
        }

    LOG_FUNCTION_NAME_EXIT;

    return ErrorUtils::omxToAndroidError(eError);
}

status_t OMXCameraAdapter::setWBMode(Gen3A_settings& Gen3A)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_CONFIG_WHITEBALCONTROLTYPE wb;

    LOG_FUNCTION_NAME;

    if ( OMX_StateInvalid == mComponentState )
        {
        CAMHAL_LOGEA("OMX component is in invalid state");
        return NO_INIT;
        }

    OMX_INIT_STRUCT_PTR (&wb, OMX_CONFIG_WHITEBALCONTROLTYPE);
    wb.nPortIndex = mCameraAdapterParameters.mPrevPortIndex;
    wb.eWhiteBalControl = ( OMX_WHITEBALCONTROLTYPE ) Gen3A.WhiteBallance;

    if ( WB_FACE_PRIORITY == Gen3A.WhiteBallance )
        {
        //Disable Region priority and enable Face priority
        setAlgoPriority(REGION_PRIORITY, WHITE_BALANCE_ALGO, false);
        setAlgoPriority(FACE_PRIORITY, WHITE_BALANCE_ALGO, true);

        //Then set the mode to auto
        wb.eWhiteBalControl = OMX_WhiteBalControlAuto;
        }
    else
        {
        //Disable Face and Region priority
        setAlgoPriority(FACE_PRIORITY, WHITE_BALANCE_ALGO, false);
        setAlgoPriority(REGION_PRIORITY, WHITE_BALANCE_ALGO, false);
        }

    eError = OMX_SetConfig( mCameraAdapterParameters.mHandleComp,
                         OMX_IndexConfigCommonWhiteBalance,
                         &wb);
    if ( OMX_ErrorNone != eError )
        {
        CAMHAL_LOGEB("Error while configuring Whitebalance mode 0x%x error = 0x%x",
                     ( unsigned int ) wb.eWhiteBalControl,
                     eError);
        }
    else
        {
        CAMHAL_LOGDB("Whitebalance mode 0x%x configured successfully",
                     ( unsigned int ) wb.eWhiteBalControl);
        }

    LOG_FUNCTION_NAME_EXIT;

    return eError;
}

status_t OMXCameraAdapter::setFlicker(Gen3A_settings& Gen3A)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_CONFIG_FLICKERCANCELTYPE flicker;

    LOG_FUNCTION_NAME;

    if ( OMX_StateInvalid == mComponentState )
        {
        CAMHAL_LOGEA("OMX component is in invalid state");
        return NO_INIT;
        }

    OMX_INIT_STRUCT_PTR (&flicker, OMX_CONFIG_FLICKERCANCELTYPE);
    flicker.nPortIndex = mCameraAdapterParameters.mPrevPortIndex;
    flicker.eFlickerCancel = (OMX_COMMONFLICKERCANCELTYPE)Gen3A.Flicker;

    eError = OMX_SetConfig( mCameraAdapterParameters.mHandleComp,
                            (OMX_INDEXTYPE)OMX_IndexConfigFlickerCancel,
                            &flicker );
    if ( OMX_ErrorNone != eError )
        {
        CAMHAL_LOGEB("Error while configuring Flicker mode 0x%x error = 0x%x",
                     ( unsigned int ) flicker.eFlickerCancel,
                     eError);
        }
    else
        {
        CAMHAL_LOGDB("Flicker mode 0x%x configured successfully",
                     ( unsigned int ) flicker.eFlickerCancel);
        }

    LOG_FUNCTION_NAME_EXIT;

    return ErrorUtils::omxToAndroidError(eError);
}

status_t OMXCameraAdapter::setBrightness(Gen3A_settings& Gen3A)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_CONFIG_BRIGHTNESSTYPE brightness;

    LOG_FUNCTION_NAME;

    if ( OMX_StateInvalid == mComponentState )
        {
        CAMHAL_LOGEA("OMX component is in invalid state");
        return NO_INIT;
        }

    OMX_INIT_STRUCT_PTR (&brightness, OMX_CONFIG_BRIGHTNESSTYPE);
    brightness.nPortIndex = mCameraAdapterParameters.mPrevPortIndex;
    brightness.nBrightness = Gen3A.Brightness;

    eError = OMX_SetConfig( mCameraAdapterParameters.mHandleComp,
                         OMX_IndexConfigCommonBrightness,
                         &brightness);
    if ( OMX_ErrorNone != eError )
        {
        CAMHAL_LOGEB("Error while configuring Brightness 0x%x error = 0x%x",
                     ( unsigned int ) brightness.nBrightness,
                     eError);
        }
    else
        {
        CAMHAL_LOGDB("Brightness 0x%x configured successfully",
                     ( unsigned int ) brightness.nBrightness);
        }

    LOG_FUNCTION_NAME_EXIT;

    return ErrorUtils::omxToAndroidError(eError);
}

status_t OMXCameraAdapter::setContrast(Gen3A_settings& Gen3A)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_CONFIG_CONTRASTTYPE contrast;

    LOG_FUNCTION_NAME;

    if ( OMX_StateInvalid == mComponentState )
        {
        CAMHAL_LOGEA("OMX component is in invalid state");
        return NO_INIT;
        }

    OMX_INIT_STRUCT_PTR (&contrast, OMX_CONFIG_CONTRASTTYPE);
    contrast.nPortIndex = mCameraAdapterParameters.mPrevPortIndex;
    contrast.nContrast = Gen3A.Contrast;

    eError = OMX_SetConfig( mCameraAdapterParameters.mHandleComp,
                         OMX_IndexConfigCommonContrast,
                         &contrast);
    if ( OMX_ErrorNone != eError )
        {
        CAMHAL_LOGEB("Error while configuring Contrast 0x%x error = 0x%x",
                     ( unsigned int ) contrast.nContrast,
                     eError);
        }
    else
        {
        CAMHAL_LOGDB("Contrast 0x%x configured successfully",
                     ( unsigned int ) contrast.nContrast);
        }

    LOG_FUNCTION_NAME_EXIT;

    return eError;
}

status_t OMXCameraAdapter::setSharpness(Gen3A_settings& Gen3A)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_IMAGE_CONFIG_PROCESSINGLEVELTYPE procSharpness;

    LOG_FUNCTION_NAME;

    if ( OMX_StateInvalid == mComponentState )
        {
        CAMHAL_LOGEA("OMX component is in invalid state");
        return NO_INIT;
        }

    OMX_INIT_STRUCT_PTR (&procSharpness, OMX_IMAGE_CONFIG_PROCESSINGLEVELTYPE);
    procSharpness.nPortIndex = mCameraAdapterParameters.mPrevPortIndex;
    procSharpness.nLevel = Gen3A.Sharpness;

    if( procSharpness.nLevel == 0 )
        {
        procSharpness.bAuto = OMX_TRUE;
        }
    else
        {
        procSharpness.bAuto = OMX_FALSE;
        }

    eError = OMX_SetConfig( mCameraAdapterParameters.mHandleComp,
                         (OMX_INDEXTYPE)OMX_IndexConfigSharpeningLevel,
                         &procSharpness);
    if ( OMX_ErrorNone != eError )
        {
        CAMHAL_LOGEB("Error while configuring Sharpness 0x%x error = 0x%x",
                     ( unsigned int ) procSharpness.nLevel,
                     eError);
        }
    else
        {
        CAMHAL_LOGDB("Sharpness 0x%x configured successfully",
                     ( unsigned int ) procSharpness.nLevel);
        }

    LOG_FUNCTION_NAME_EXIT;

    return ErrorUtils::omxToAndroidError(eError);
}

status_t OMXCameraAdapter::setSaturation(Gen3A_settings& Gen3A)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_CONFIG_SATURATIONTYPE saturation;

    LOG_FUNCTION_NAME;

    if ( OMX_StateInvalid == mComponentState )
        {
        CAMHAL_LOGEA("OMX component is in invalid state");
        return NO_INIT;
        }

    OMX_INIT_STRUCT_PTR (&saturation, OMX_CONFIG_SATURATIONTYPE);
    saturation.nPortIndex = mCameraAdapterParameters.mPrevPortIndex;
    saturation.nSaturation = Gen3A.Saturation;

    eError = OMX_SetConfig( mCameraAdapterParameters.mHandleComp,
                         OMX_IndexConfigCommonSaturation,
                         &saturation);
    if ( OMX_ErrorNone != eError )
        {
        CAMHAL_LOGEB("Error while configuring Saturation 0x%x error = 0x%x",
                     ( unsigned int ) saturation.nSaturation,
                     eError);
        }
    else
        {
        CAMHAL_LOGDB("Saturation 0x%x configured successfully",
                     ( unsigned int ) saturation.nSaturation);
        }

    LOG_FUNCTION_NAME_EXIT;

    return ErrorUtils::omxToAndroidError(eError);
}

status_t OMXCameraAdapter::setISO(Gen3A_settings& Gen3A)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_CONFIG_EXPOSUREVALUETYPE expValues;

    LOG_FUNCTION_NAME;

    if ( OMX_StateInvalid == mComponentState )
        {
        CAMHAL_LOGEA("OMX component is in invalid state");
        return NO_INIT;
        }

    OMX_INIT_STRUCT_PTR (&expValues, OMX_CONFIG_EXPOSUREVALUETYPE);
    expValues.nPortIndex = mCameraAdapterParameters.mPrevPortIndex;

    OMX_GetConfig( mCameraAdapterParameters.mHandleComp,
                   OMX_IndexConfigCommonExposureValue,
                   &expValues);

    if( 0 == Gen3A.ISO )
        {
        expValues.bAutoSensitivity = OMX_TRUE;
        }
    else
        {
        expValues.bAutoSensitivity = OMX_FALSE;
        expValues.nSensitivity = Gen3A.ISO;
        }

    eError = OMX_SetConfig( mCameraAdapterParameters.mHandleComp,
                         OMX_IndexConfigCommonExposureValue,
                         &expValues);
    if ( OMX_ErrorNone != eError )
        {
        CAMHAL_LOGEB("Error while configuring ISO 0x%x error = 0x%x",
                     ( unsigned int ) expValues.nSensitivity,
                     eError);
        }
    else
        {
        CAMHAL_LOGDB("ISO 0x%x configured successfully",
                     ( unsigned int ) expValues.nSensitivity);
        }

    LOG_FUNCTION_NAME_EXIT;

    return ErrorUtils::omxToAndroidError(eError);
}

status_t OMXCameraAdapter::setEffect(Gen3A_settings& Gen3A)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_CONFIG_IMAGEFILTERTYPE effect;

    LOG_FUNCTION_NAME;

    if ( OMX_StateInvalid == mComponentState )
        {
        CAMHAL_LOGEA("OMX component is in invalid state");
        return NO_INIT;
        }

    OMX_INIT_STRUCT_PTR (&effect, OMX_CONFIG_IMAGEFILTERTYPE);
    effect.nPortIndex = mCameraAdapterParameters.mPrevPortIndex;
    effect.eImageFilter = (OMX_IMAGEFILTERTYPE ) Gen3A.Effect;

    eError = OMX_SetConfig( mCameraAdapterParameters.mHandleComp,
                         OMX_IndexConfigCommonImageFilter,
                         &effect);
    if ( OMX_ErrorNone != eError )
        {
        CAMHAL_LOGEB("Error while configuring Effect 0x%x error = 0x%x",
                     ( unsigned int )  effect.eImageFilter,
                     eError);
        }
    else
        {
        CAMHAL_LOGDB("Effect 0x%x configured successfully",
                     ( unsigned int )  effect.eImageFilter);
        }

    LOG_FUNCTION_NAME_EXIT;

    return ErrorUtils::omxToAndroidError(eError);
}

status_t OMXCameraAdapter::setWhiteBalanceLock(Gen3A_settings& Gen3A)
{
  OMX_ERRORTYPE eError = OMX_ErrorNone;
  OMX_IMAGE_CONFIG_LOCKTYPE lock;

  LOG_FUNCTION_NAME

  if ( OMX_StateInvalid == mComponentState )
    {
      CAMHAL_LOGEA("OMX component is in invalid state");
      return NO_INIT;
    }

  OMX_INIT_STRUCT_PTR (&lock, OMX_IMAGE_CONFIG_LOCKTYPE);
  lock.nPortIndex = mCameraAdapterParameters.mPrevPortIndex;
  lock.bLock = Gen3A.WhiteBalanceLock;
  eError = OMX_SetConfig( mCameraAdapterParameters.mHandleComp,
                          (OMX_INDEXTYPE)OMX_IndexConfigImageWhiteBalanceLock,
                          &lock);
  if ( OMX_ErrorNone != eError )
    {
      CAMHAL_LOGEB("Error while configuring WhiteBalance Lock error = 0x%x", eError);
    }
  else
    {
      CAMHAL_LOGDB("WhiteBalance Lock configured successfully %d ", lock.bLock);
    }
  LOG_FUNCTION_NAME_EXIT

  return ErrorUtils::omxToAndroidError(eError);
}

status_t OMXCameraAdapter::setExposureLock(Gen3A_settings& Gen3A)
{
  OMX_ERRORTYPE eError = OMX_ErrorNone;
  OMX_IMAGE_CONFIG_LOCKTYPE lock;

  LOG_FUNCTION_NAME

  if ( OMX_StateInvalid == mComponentState )
    {
      CAMHAL_LOGEA("OMX component is in invalid state");
      return NO_INIT;
    }

  OMX_INIT_STRUCT_PTR (&lock, OMX_IMAGE_CONFIG_LOCKTYPE);
  lock.nPortIndex = mCameraAdapterParameters.mPrevPortIndex;
  lock.bLock = Gen3A.ExposureLock;
  eError = OMX_SetConfig( mCameraAdapterParameters.mHandleComp,
                          (OMX_INDEXTYPE)OMX_IndexConfigImageExposureLock,
                          &lock);
  if ( OMX_ErrorNone != eError )
    {
      CAMHAL_LOGEB("Error while configuring Exposure Lock error = 0x%x", eError);
    }
  else
    {
      CAMHAL_LOGDB("Exposure Lock configured successfully %d ", lock.bLock);
    }
  LOG_FUNCTION_NAME_EXIT

    return ErrorUtils::omxToAndroidError(eError);
}

status_t OMXCameraAdapter::setFocusLock(Gen3A_settings& Gen3A)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_IMAGE_CONFIG_LOCKTYPE lock;

    LOG_FUNCTION_NAME

    if ( OMX_StateInvalid == mComponentState ) {
        CAMHAL_LOGEA("OMX component is in invalid state");
        return NO_INIT;
    }

    OMX_INIT_STRUCT_PTR (&lock, OMX_IMAGE_CONFIG_LOCKTYPE);
    lock.nPortIndex = mCameraAdapterParameters.mPrevPortIndex;

    lock.bLock = Gen3A.FocusLock;
    eError = OMX_SetConfig(mCameraAdapterParameters.mHandleComp,
                           (OMX_INDEXTYPE)OMX_IndexConfigImageFocusLock,
                           &lock);

    if ( OMX_ErrorNone != eError ) {
        CAMHAL_LOGEB("Error while configuring Focus Lock error = 0x%x", eError);
    } else {
        CAMHAL_LOGDB("Focus Lock configured successfully %d ", lock.bLock);
    }

    LOG_FUNCTION_NAME_EXIT

    return ErrorUtils::omxToAndroidError(eError);
}

status_t OMXCameraAdapter::set3ALock(OMX_BOOL toggleExp, OMX_BOOL toggleWb, OMX_BOOL toggleFocus)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_IMAGE_CONFIG_LOCKTYPE lock;

    LOG_FUNCTION_NAME

    if ( OMX_StateInvalid == mComponentState )
    {
      CAMHAL_LOGEA("OMX component is in invalid state");
      return NO_INIT;
    }

    OMX_INIT_STRUCT_PTR (&lock, OMX_IMAGE_CONFIG_LOCKTYPE);
    lock.nPortIndex = mCameraAdapterParameters.mPrevPortIndex;

    mParameters3A.ExposureLock = toggleExp;
    mParameters3A.FocusLock = toggleFocus;
    mParameters3A.WhiteBalanceLock = toggleWb;

    eError = OMX_GetConfig( mCameraAdapterParameters.mHandleComp,
                            (OMX_INDEXTYPE)OMX_IndexConfigImageExposureLock,
                            &lock);

    if ( OMX_ErrorNone != eError )
    {
        CAMHAL_LOGEB("Error GetConfig Exposure Lock error = 0x%x", eError);
        goto EXIT;
    }
    else
    {
        const char *lock_state_exp = toggleExp ? TRUE : FALSE;
        CAMHAL_LOGDA("Exposure Lock GetConfig successfull");

        /* Apply locks only when not applied already */
        if ( lock.bLock  != toggleExp )
        {
            setExposureLock(mParameters3A);
        }

        mParams.set(CameraParameters::KEY_AUTO_EXPOSURE_LOCK, lock_state_exp);
    }

    OMX_INIT_STRUCT_PTR (&lock, OMX_IMAGE_CONFIG_LOCKTYPE);
    lock.nPortIndex = mCameraAdapterParameters.mPrevPortIndex;
    eError = OMX_GetConfig( mCameraAdapterParameters.mHandleComp,
                            (OMX_INDEXTYPE)OMX_IndexConfigImageFocusLock,
                            &lock);

    if ( OMX_ErrorNone != eError )
    {
        CAMHAL_LOGEB("Error GetConfig Focus Lock error = 0x%x", eError);
        goto EXIT;
    }
    else
    {
        CAMHAL_LOGDB("Focus Lock GetConfig successfull bLock(%d)", lock.bLock);

        /* Apply locks only when not applied already */
        if ( lock.bLock  != toggleFocus )
        {
            setFocusLock(mParameters3A);
        }
    }

    OMX_INIT_STRUCT_PTR (&lock, OMX_IMAGE_CONFIG_LOCKTYPE);
    lock.nPortIndex = mCameraAdapterParameters.mPrevPortIndex;
    eError = OMX_GetConfig( mCameraAdapterParameters.mHandleComp,
                            (OMX_INDEXTYPE)OMX_IndexConfigImageWhiteBalanceLock,
                            &lock);

    if ( OMX_ErrorNone != eError )
    {
        CAMHAL_LOGEB("Error GetConfig WhiteBalance Lock error = 0x%x", eError);
        goto EXIT;
    }
    else
    {
        const char *lock_state_wb = toggleWb ? TRUE : FALSE;
        CAMHAL_LOGDA("WhiteBalance Lock GetConfig successfull");

        /* Apply locks only when not applied already */
        if ( lock.bLock != toggleWb )
        {
            setWhiteBalanceLock(mParameters3A);
        }

        mParams.set(CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK, lock_state_wb);
    }
 EXIT:
    return ErrorUtils::omxToAndroidError(eError);
}

status_t OMXCameraAdapter::setMeteringAreas(Gen3A_settings& Gen3A)
{
  status_t ret = NO_ERROR;
  OMX_ERRORTYPE eError = OMX_ErrorNone;

  OMX_ALGOAREASTYPE **meteringAreas;
  OMX_TI_CONFIG_SHAREDBUFFER sharedBuffer;
  MemoryManager memMgr;
  int areasSize = 0;

  LOG_FUNCTION_NAME

  if ( OMX_StateInvalid == mComponentState )
    {
      CAMHAL_LOGEA("OMX component is in invalid state");
      return NO_INIT;
    }

  areasSize = ((sizeof(OMX_ALGOAREASTYPE)+4095)/4096)*4096;
  meteringAreas = (OMX_ALGOAREASTYPE**) memMgr.allocateBuffer(0, 0, NULL, areasSize, 1);

  OMXCameraPortParameters * mPreviewData = NULL;
  mPreviewData = &mCameraAdapterParameters.mCameraPortParams[mCameraAdapterParameters.mPrevPortIndex];

  if (!meteringAreas)
      {
      CAMHAL_LOGEB("Error allocating buffer for metering areas %d", eError);
      return -ENOMEM;
      }

  OMX_INIT_STRUCT_PTR (meteringAreas[0], OMX_ALGOAREASTYPE);

  meteringAreas[0]->nPortIndex = OMX_ALL;
  meteringAreas[0]->nNumAreas = mMeteringAreas.size();
  meteringAreas[0]->nAlgoAreaPurpose = OMX_AlgoAreaExposure;

  for ( unsigned int n = 0; n < mMeteringAreas.size(); n++)
      {
      // transform the coordinates to 3A-type coordinates
      mMeteringAreas.itemAt(n)->transfrom((size_t)mPreviewData->mWidth,
                                      (size_t)mPreviewData->mHeight,
                                      (size_t&)meteringAreas[0]->tAlgoAreas[n].nTop,
                                      (size_t&)meteringAreas[0]->tAlgoAreas[n].nLeft,
                                      (size_t&)meteringAreas[0]->tAlgoAreas[n].nWidth,
                                      (size_t&)meteringAreas[0]->tAlgoAreas[n].nHeight);

      meteringAreas[0]->tAlgoAreas[n].nLeft =
              ( meteringAreas[0]->tAlgoAreas[n].nLeft * METERING_AREAS_RANGE ) / mPreviewData->mWidth;
      meteringAreas[0]->tAlgoAreas[n].nTop =
              ( meteringAreas[0]->tAlgoAreas[n].nTop* METERING_AREAS_RANGE ) / mPreviewData->mHeight;
      meteringAreas[0]->tAlgoAreas[n].nWidth =
              ( meteringAreas[0]->tAlgoAreas[n].nWidth * METERING_AREAS_RANGE ) / mPreviewData->mWidth;
      meteringAreas[0]->tAlgoAreas[n].nHeight =
              ( meteringAreas[0]->tAlgoAreas[n].nHeight * METERING_AREAS_RANGE ) / mPreviewData->mHeight;

      meteringAreas[0]->tAlgoAreas[n].nPriority = mMeteringAreas.itemAt(n)->getWeight();

      CAMHAL_LOGDB("Metering area %d : top = %d left = %d width = %d height = %d prio = %d",
              n, (int)meteringAreas[0]->tAlgoAreas[n].nTop, (int)meteringAreas[0]->tAlgoAreas[n].nLeft,
              (int)meteringAreas[0]->tAlgoAreas[n].nWidth, (int)meteringAreas[0]->tAlgoAreas[n].nHeight,
              (int)meteringAreas[0]->tAlgoAreas[n].nPriority);

      }

  OMX_INIT_STRUCT_PTR (&sharedBuffer, OMX_TI_CONFIG_SHAREDBUFFER);

  sharedBuffer.nPortIndex = OMX_ALL;
  sharedBuffer.nSharedBuffSize = areasSize;
  sharedBuffer.pSharedBuff = (OMX_U8 *) meteringAreas[0];

  if ( NULL == sharedBuffer.pSharedBuff )
      {
      CAMHAL_LOGEA("No resources to allocate OMX shared buffer");
      ret = -ENOMEM;
      goto EXIT;
      }

      eError =  OMX_SetConfig(mCameraAdapterParameters.mHandleComp,
                                (OMX_INDEXTYPE) OMX_TI_IndexConfigAlgoAreas, &sharedBuffer);

  if ( OMX_ErrorNone != eError )
      {
      CAMHAL_LOGEB("Error while setting Focus Areas configuration 0x%x", eError);
      ret = -EINVAL;
      }
  else
      {
      CAMHAL_LOGDA("Metering Areas SetConfig successfull.");
      }

 EXIT:
  if (NULL != meteringAreas)
      {
      memMgr.freeBuffer((void*) meteringAreas);
      meteringAreas = NULL;
      }

  return ret;
}

status_t OMXCameraAdapter::apply3Asettings( Gen3A_settings& Gen3A )
{
    status_t ret = NO_ERROR;
    unsigned int currSett; // 32 bit
    int portIndex;

    /*
     * Scenes have a priority during the process
     * of applying 3A related parameters.
     * They can override pretty much all other 3A
     * settings and similarly get overridden when
     * for instance the focus mode gets switched.
     * There is only one exception to this rule,
     * the manual a.k.a. auto scene.
     */
    if ( ( SetSceneMode & mPending3Asettings ) )
        {
        mPending3Asettings &= ~SetSceneMode;
        return setScene(Gen3A);
        }
    else if ( OMX_Manual != Gen3A.SceneMode )
        {
        mPending3Asettings = 0;
        return NO_ERROR;
        }

    for( currSett = 1; currSett < E3aSettingMax; currSett <<= 1)
        {
        if( currSett & mPending3Asettings )
            {
            switch( currSett )
                {
                case SetEVCompensation:
                    {
                    ret |= setEVCompensation(Gen3A);
                    break;
                    }

                case SetWhiteBallance:
                    {
                    ret |= setWBMode(Gen3A);
                    break;
                    }

                case SetFlicker:
                    {
                    ret |= setFlicker(Gen3A);
                    break;
                    }

                case SetBrightness:
                    {
                    ret |= setBrightness(Gen3A);
                    break;
                    }

                case SetContrast:
                    {
                    ret |= setContrast(Gen3A);
                    break;
                    }

                case SetSharpness:
                    {
                    ret |= setSharpness(Gen3A);
                    break;
                    }

                case SetSaturation:
                    {
                    ret |= setSaturation(Gen3A);
                    break;
                    }

                case SetISO:
                    {
                    ret |= setISO(Gen3A);
                    break;
                    }

                case SetEffect:
                    {
                    ret |= setEffect(Gen3A);
                    break;
                    }

                case SetFocus:
                    {
                    ret |= setFocusMode(Gen3A);
                    break;
                    }

                case SetExpMode:
                    {
                    ret |= setExposureMode(Gen3A);
                    break;
                    }

                case SetFlash:
                    {
                    ret |= setFlashMode(Gen3A);
                    break;
                    }

                case SetExpLock:
                  {
                    ret |= setExposureLock(Gen3A);
                    break;
                  }

                case SetWBLock:
                  {
                    ret |= setWhiteBalanceLock(Gen3A);
                    break;
                  }
                case SetMeteringAreas:
                  {
                    ret |= setMeteringAreas(Gen3A);
                  }
                  break;
                default:
                    CAMHAL_LOGEB("this setting (0x%x) is still not supported in CameraAdapter ",
                                 currSett);
                    break;
                }
                mPending3Asettings &= ~currSett;
            }
        }
        return ret;
}

};
