/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (https://www.swig.org).
 * Version 4.1.1
 *
 * Do not make changes to this file unless you know what you are doing - modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.taptap.tapsdk.bindings.java;

public class Window {
  private transient long swigCPtr;
  private transient boolean swigCMemOwn;

  protected Window(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(Window obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void swigSetCMemOwn(boolean own) {
    swigCMemOwn = own;
  }

  @SuppressWarnings("deprecation")
  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        PlatformJNI.delete_Window(swigCPtr);
      }
      swigCPtr = 0;
    }
  }

  public static void OnForeground() {
    PlatformJNI.Window_OnForeground();
  }

  public static void OnBackground() {
    PlatformJNI.Window_OnBackground();
  }

  public Window() {
    this(PlatformJNI.new_Window(), true);
  }

}