//------------------------------------------------------------------------------
// <auto-generated />
//
// This file was automatically generated by SWIG (https://www.swig.org).
// Version 4.1.1
//
// Do not make changes to this file unless you know what you are doing - modify
// the SWIG interface file instead.
//------------------------------------------------------------------------------

namespace com.taptap.tapsdk.bindings.csharp {

class TapSDKPINVOKE {

  protected class SWIGExceptionHelper {

    public delegate void ExceptionDelegate(string message);
    public delegate void ExceptionArgumentDelegate(string message, string paramName);

    static ExceptionDelegate applicationDelegate = new ExceptionDelegate(SetPendingApplicationException);
    static ExceptionDelegate arithmeticDelegate = new ExceptionDelegate(SetPendingArithmeticException);
    static ExceptionDelegate divideByZeroDelegate = new ExceptionDelegate(SetPendingDivideByZeroException);
    static ExceptionDelegate indexOutOfRangeDelegate = new ExceptionDelegate(SetPendingIndexOutOfRangeException);
    static ExceptionDelegate invalidCastDelegate = new ExceptionDelegate(SetPendingInvalidCastException);
    static ExceptionDelegate invalidOperationDelegate = new ExceptionDelegate(SetPendingInvalidOperationException);
    static ExceptionDelegate ioDelegate = new ExceptionDelegate(SetPendingIOException);
    static ExceptionDelegate nullReferenceDelegate = new ExceptionDelegate(SetPendingNullReferenceException);
    static ExceptionDelegate outOfMemoryDelegate = new ExceptionDelegate(SetPendingOutOfMemoryException);
    static ExceptionDelegate overflowDelegate = new ExceptionDelegate(SetPendingOverflowException);
    static ExceptionDelegate systemDelegate = new ExceptionDelegate(SetPendingSystemException);

    static ExceptionArgumentDelegate argumentDelegate = new ExceptionArgumentDelegate(SetPendingArgumentException);
    static ExceptionArgumentDelegate argumentNullDelegate = new ExceptionArgumentDelegate(SetPendingArgumentNullException);
    static ExceptionArgumentDelegate argumentOutOfRangeDelegate = new ExceptionArgumentDelegate(SetPendingArgumentOutOfRangeException);

    [global::System.Runtime.InteropServices.DllImport("libbindings-csharp", EntryPoint="SWIGRegisterExceptionCallbacks_TapSDK")]
    public static extern void SWIGRegisterExceptionCallbacks_TapSDK(
                                ExceptionDelegate applicationDelegate,
                                ExceptionDelegate arithmeticDelegate,
                                ExceptionDelegate divideByZeroDelegate, 
                                ExceptionDelegate indexOutOfRangeDelegate, 
                                ExceptionDelegate invalidCastDelegate,
                                ExceptionDelegate invalidOperationDelegate,
                                ExceptionDelegate ioDelegate,
                                ExceptionDelegate nullReferenceDelegate,
                                ExceptionDelegate outOfMemoryDelegate, 
                                ExceptionDelegate overflowDelegate, 
                                ExceptionDelegate systemExceptionDelegate);

    [global::System.Runtime.InteropServices.DllImport("libbindings-csharp", EntryPoint="SWIGRegisterExceptionArgumentCallbacks_TapSDK")]
    public static extern void SWIGRegisterExceptionCallbacksArgument_TapSDK(
                                ExceptionArgumentDelegate argumentDelegate,
                                ExceptionArgumentDelegate argumentNullDelegate,
                                ExceptionArgumentDelegate argumentOutOfRangeDelegate);

    static void SetPendingApplicationException(string message) {
      SWIGPendingException.Set(new global::System.ApplicationException(message, SWIGPendingException.Retrieve()));
    }
    static void SetPendingArithmeticException(string message) {
      SWIGPendingException.Set(new global::System.ArithmeticException(message, SWIGPendingException.Retrieve()));
    }
    static void SetPendingDivideByZeroException(string message) {
      SWIGPendingException.Set(new global::System.DivideByZeroException(message, SWIGPendingException.Retrieve()));
    }
    static void SetPendingIndexOutOfRangeException(string message) {
      SWIGPendingException.Set(new global::System.IndexOutOfRangeException(message, SWIGPendingException.Retrieve()));
    }
    static void SetPendingInvalidCastException(string message) {
      SWIGPendingException.Set(new global::System.InvalidCastException(message, SWIGPendingException.Retrieve()));
    }
    static void SetPendingInvalidOperationException(string message) {
      SWIGPendingException.Set(new global::System.InvalidOperationException(message, SWIGPendingException.Retrieve()));
    }
    static void SetPendingIOException(string message) {
      SWIGPendingException.Set(new global::System.IO.IOException(message, SWIGPendingException.Retrieve()));
    }
    static void SetPendingNullReferenceException(string message) {
      SWIGPendingException.Set(new global::System.NullReferenceException(message, SWIGPendingException.Retrieve()));
    }
    static void SetPendingOutOfMemoryException(string message) {
      SWIGPendingException.Set(new global::System.OutOfMemoryException(message, SWIGPendingException.Retrieve()));
    }
    static void SetPendingOverflowException(string message) {
      SWIGPendingException.Set(new global::System.OverflowException(message, SWIGPendingException.Retrieve()));
    }
    static void SetPendingSystemException(string message) {
      SWIGPendingException.Set(new global::System.SystemException(message, SWIGPendingException.Retrieve()));
    }

    static void SetPendingArgumentException(string message, string paramName) {
      SWIGPendingException.Set(new global::System.ArgumentException(message, paramName, SWIGPendingException.Retrieve()));
    }
    static void SetPendingArgumentNullException(string message, string paramName) {
      global::System.Exception e = SWIGPendingException.Retrieve();
      if (e != null) message = message + " Inner Exception: " + e.Message;
      SWIGPendingException.Set(new global::System.ArgumentNullException(paramName, message));
    }
    static void SetPendingArgumentOutOfRangeException(string message, string paramName) {
      global::System.Exception e = SWIGPendingException.Retrieve();
      if (e != null) message = message + " Inner Exception: " + e.Message;
      SWIGPendingException.Set(new global::System.ArgumentOutOfRangeException(paramName, message));
    }

    static SWIGExceptionHelper() {
      SWIGRegisterExceptionCallbacks_TapSDK(
                                applicationDelegate,
                                arithmeticDelegate,
                                divideByZeroDelegate,
                                indexOutOfRangeDelegate,
                                invalidCastDelegate,
                                invalidOperationDelegate,
                                ioDelegate,
                                nullReferenceDelegate,
                                outOfMemoryDelegate,
                                overflowDelegate,
                                systemDelegate);

      SWIGRegisterExceptionCallbacksArgument_TapSDK(
                                argumentDelegate,
                                argumentNullDelegate,
                                argumentOutOfRangeDelegate);
    }
  }

  protected static SWIGExceptionHelper swigExceptionHelper = new SWIGExceptionHelper();

  public class SWIGPendingException {
    [global::System.ThreadStatic]
    private static global::System.Exception pendingException = null;
    private static int numExceptionsPending = 0;
    private static global::System.Object exceptionsLock = null;

    public static bool Pending {
      get {
        bool pending = false;
        if (numExceptionsPending > 0)
          if (pendingException != null)
            pending = true;
        return pending;
      } 
    }

    public static void Set(global::System.Exception e) {
      if (pendingException != null)
        throw new global::System.ApplicationException("FATAL: An earlier pending exception from unmanaged code was missed and thus not thrown (" + pendingException.ToString() + ")", e);
      pendingException = e;
      lock(exceptionsLock) {
        numExceptionsPending++;
      }
    }

    public static global::System.Exception Retrieve() {
      global::System.Exception e = null;
      if (numExceptionsPending > 0) {
        if (pendingException != null) {
          e = pendingException;
          pendingException = null;
          lock(exceptionsLock) {
            numExceptionsPending--;
          }
        }
      }
      return e;
    }

    static SWIGPendingException() {
      exceptionsLock = new global::System.Object();
    }
  }


  protected class SWIGStringHelper {

    public delegate string SWIGStringDelegate(string message);
    static SWIGStringDelegate stringDelegate = new SWIGStringDelegate(CreateString);

    [global::System.Runtime.InteropServices.DllImport("libbindings-csharp", EntryPoint="SWIGRegisterStringCallback_TapSDK")]
    public static extern void SWIGRegisterStringCallback_TapSDK(SWIGStringDelegate stringDelegate);

    static string CreateString(string cString) {
      return cString;
    }

    static SWIGStringHelper() {
      SWIGRegisterStringCallback_TapSDK(stringDelegate);
    }
  }

  static protected SWIGStringHelper swigStringHelper = new SWIGStringHelper();


  static TapSDKPINVOKE() {
  }


  [global::System.Runtime.InteropServices.DllImport("libbindings-csharp", EntryPoint="CSharp_comftaptapftapsdkfbindingsfcsharp_TDSUser_SetCurrent___")]
  public static extern void TDSUser_SetCurrent(global::System.Runtime.InteropServices.HandleRef jarg1);

  [global::System.Runtime.InteropServices.DllImport("libbindings-csharp", EntryPoint="CSharp_comftaptapftapsdkfbindingsfcsharp_TDSUser_GetCurrent___")]
  public static extern global::System.IntPtr TDSUser_GetCurrent();

  [global::System.Runtime.InteropServices.DllImport("libbindings-csharp", EntryPoint="CSharp_comftaptapftapsdkfbindingsfcsharp_new_TDSUser__SWIG_0___")]
  public static extern global::System.IntPtr new_TDSUser__SWIG_0(string jarg1);

  [global::System.Runtime.InteropServices.DllImport("libbindings-csharp", EntryPoint="CSharp_comftaptapftapsdkfbindingsfcsharp_new_TDSUser__SWIG_1___")]
  public static extern global::System.IntPtr new_TDSUser__SWIG_1();

  [global::System.Runtime.InteropServices.DllImport("libbindings-csharp", EntryPoint="CSharp_comftaptapftapsdkfbindingsfcsharp_TDSUser_GetUserId___")]
  public static extern string TDSUser_GetUserId(global::System.Runtime.InteropServices.HandleRef jarg1);

  [global::System.Runtime.InteropServices.DllImport("libbindings-csharp", EntryPoint="CSharp_comftaptapftapsdkfbindingsfcsharp_TDSUser_GetUserIdSwigExplicitTDSUser___")]
  public static extern string TDSUser_GetUserIdSwigExplicitTDSUser(global::System.Runtime.InteropServices.HandleRef jarg1);

  [global::System.Runtime.InteropServices.DllImport("libbindings-csharp", EntryPoint="CSharp_comftaptapftapsdkfbindingsfcsharp_TDSUser_GetUserName___")]
  public static extern string TDSUser_GetUserName(global::System.Runtime.InteropServices.HandleRef jarg1);

  [global::System.Runtime.InteropServices.DllImport("libbindings-csharp", EntryPoint="CSharp_comftaptapftapsdkfbindingsfcsharp_TDSUser_ContainTapInfo___")]
  public static extern bool TDSUser_ContainTapInfo(global::System.Runtime.InteropServices.HandleRef jarg1);

  [global::System.Runtime.InteropServices.DllImport("libbindings-csharp", EntryPoint="CSharp_comftaptapftapsdkfbindingsfcsharp_delete_TDSUser___")]
  public static extern void delete_TDSUser(global::System.Runtime.InteropServices.HandleRef jarg1);

  [global::System.Runtime.InteropServices.DllImport("libbindings-csharp", EntryPoint="CSharp_comftaptapftapsdkfbindingsfcsharp_TDSUser_director_connect___")]
  public static extern void TDSUser_director_connect(global::System.Runtime.InteropServices.HandleRef jarg1, TDSUser.SwigDelegateTDSUser_0 delegate0, TDSUser.SwigDelegateTDSUser_1 delegate1, TDSUser.SwigDelegateTDSUser_2 delegate2);

  [global::System.Runtime.InteropServices.DllImport("libbindings-csharp", EntryPoint="CSharp_comftaptapftapsdkfbindingsfcsharp_Game_SetCurrent___")]
  public static extern void Game_SetCurrent(global::System.Runtime.InteropServices.HandleRef jarg1);

  [global::System.Runtime.InteropServices.DllImport("libbindings-csharp", EntryPoint="CSharp_comftaptapftapsdkfbindingsfcsharp_Game_GetCurrent___")]
  public static extern global::System.IntPtr Game_GetCurrent();

  [global::System.Runtime.InteropServices.DllImport("libbindings-csharp", EntryPoint="CSharp_comftaptapftapsdkfbindingsfcsharp_Game_GetGameID___")]
  public static extern string Game_GetGameID(global::System.Runtime.InteropServices.HandleRef jarg1);

  [global::System.Runtime.InteropServices.DllImport("libbindings-csharp", EntryPoint="CSharp_comftaptapftapsdkfbindingsfcsharp_Game_GetPackageName___")]
  public static extern string Game_GetPackageName(global::System.Runtime.InteropServices.HandleRef jarg1);

  [global::System.Runtime.InteropServices.DllImport("libbindings-csharp", EntryPoint="CSharp_comftaptapftapsdkfbindingsfcsharp_new_Game___")]
  public static extern global::System.IntPtr new_Game();

  [global::System.Runtime.InteropServices.DllImport("libbindings-csharp", EntryPoint="CSharp_comftaptapftapsdkfbindingsfcsharp_delete_Game___")]
  public static extern void delete_Game(global::System.Runtime.InteropServices.HandleRef jarg1);

  [global::System.Runtime.InteropServices.DllImport("libbindings-csharp", EntryPoint="CSharp_comftaptapftapsdkfbindingsfcsharp_Game_director_connect___")]
  public static extern void Game_director_connect(global::System.Runtime.InteropServices.HandleRef jarg1, Game.SwigDelegateGame_0 delegate0, Game.SwigDelegateGame_1 delegate1);

  [global::System.Runtime.InteropServices.DllImport("libbindings-csharp", EntryPoint="CSharp_comftaptapftapsdkfbindingsfcsharp_Config_enable_duration_statistics_set___")]
  public static extern void Config_enable_duration_statistics_set(global::System.Runtime.InteropServices.HandleRef jarg1, bool jarg2);

  [global::System.Runtime.InteropServices.DllImport("libbindings-csharp", EntryPoint="CSharp_comftaptapftapsdkfbindingsfcsharp_Config_enable_duration_statistics_get___")]
  public static extern bool Config_enable_duration_statistics_get(global::System.Runtime.InteropServices.HandleRef jarg1);

  [global::System.Runtime.InteropServices.DllImport("libbindings-csharp", EntryPoint="CSharp_comftaptapftapsdkfbindingsfcsharp_new_Config___")]
  public static extern global::System.IntPtr new_Config();

  [global::System.Runtime.InteropServices.DllImport("libbindings-csharp", EntryPoint="CSharp_comftaptapftapsdkfbindingsfcsharp_delete_Config___")]
  public static extern void delete_Config(global::System.Runtime.InteropServices.HandleRef jarg1);

  [global::System.Runtime.InteropServices.DllImport("libbindings-csharp", EntryPoint="CSharp_comftaptapftapsdkfbindingsfcsharp_Init___")]
  public static extern bool Init(global::System.Runtime.InteropServices.HandleRef jarg1);
}

}