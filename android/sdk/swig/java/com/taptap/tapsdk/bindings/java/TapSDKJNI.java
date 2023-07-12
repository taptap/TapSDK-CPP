/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (https://www.swig.org).
 * Version 4.1.1
 *
 * Do not make changes to this file unless you know what you are doing - modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.taptap.tapsdk.bindings.java;

public class TapSDKJNI {
  public final static native void TDSUser_SetCurrent(long jarg1, TDSUser jarg1_);
  public final static native long TDSUser_GetCurrent();
  public final static native String TDSUser_GetUserId(long jarg1, TDSUser jarg1_);
  public final static native String TDSUser_GetUserIdSwigExplicitTDSUser(long jarg1, TDSUser jarg1_);
  public final static native String TDSUser_GetUserName(long jarg1, TDSUser jarg1_);
  public final static native String TDSUser_GetUserNameSwigExplicitTDSUser(long jarg1, TDSUser jarg1_);
  public final static native long new_TDSUser();
  public final static native void delete_TDSUser(long jarg1);
  public final static native void TDSUser_director_connect(TDSUser obj, long cptr, boolean mem_own, boolean weak_global);
  public final static native void TDSUser_change_ownership(TDSUser obj, long cptr, boolean take_or_release);
  public final static native void Config_enable_duration_statistics_set(long jarg1, Config jarg1_, boolean jarg2);
  public final static native boolean Config_enable_duration_statistics_get(long jarg1, Config jarg1_);
  public final static native long new_Config();
  public final static native void delete_Config(long jarg1);
  public final static native boolean Init(long jarg1, Config jarg1_);

  public static String SwigDirector_TDSUser_GetUserId(TDSUser jself) {
    return jself.GetUserId();
  }
  public static String SwigDirector_TDSUser_GetUserName(TDSUser jself) {
    return jself.GetUserName();
  }

  private final static native void swig_module_init();
  static {
    swig_module_init();
  }
}
