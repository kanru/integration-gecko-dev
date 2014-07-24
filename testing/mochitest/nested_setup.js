// "use strict";

// const CHILD_SCRIPT = "chrome://specialpowers/content/specialpowers.js";
// const CHILD_SCRIPT_API = "chrome://specialpowers/content/specialpowersAPI.js";
// const CHILD_LOGGER_SCRIPT = "chrome://specialpowers/content/MozillaLogger.js";

// let specialpowers = {};
// let loader = SpecialPowers.Cc["@mozilla.org/moz/jssubscript-loader;1"]
//       .getService(SpecialPowers.Ci.mozIJSSubScriptLoader);
// loader.loadSubScript("chrome://specialpowers/content/SpecialPowersObserver.js",
//                      specialpowers);
// let specialPowersObserver = new specialpowers.SpecialPowersObserver();

// // let mm = container.QueryInterface(Ci.nsIFrameLoaderOwner).frameLoader.messageManager;
// // specialPowersObserver.init(mm);

// // mm.addMessageListener("SPPrefService", specialPowersObserver);
// // mm.addMessageListener("SPProcessCrashService", specialPowersObserver);
// // mm.addMessageListener("SPPingService", specialPowersObserver);
// // mm.addMessageListener("SpecialPowers.Quit", specialPowersObserver);
// // mm.addMessageListener("SpecialPowers.Focus", specialPowersObserver);
// // mm.addMessageListener("SPPermissionManager", specialPowersObserver);
// // mm.addMessageListener("SPLoadChromeScript", specialPowersObserver);
// // mm.addMessageListener("SPChromeScriptMessage", specialPowersObserver);

// // mm.loadFrameScript(CHILD_LOGGER_SCRIPT, true);
// // mm.loadFrameScript(CHILD_SCRIPT_API, true);
// // mm.loadFrameScript(CHILD_SCRIPT, true);

// // specialPowersObserver._isFrameScriptLoaded = true;

// // function getElement(id) {
// //   return ((typeof(id) == "string") ?
// //           document.getElementById(id) : id);
// // }

// // this.$ = this.getElement;

// // window.addEventListener("load", function(e) {
// //   var iframe = $('nested-parent-frame');
// //   var mm = SpecialPowers.wrap(iframe).QueryInterface(
// //     SpecialPowers.Ci.nsIFrameLoaderOwner).frameLoader.messageManager;
// //   mm.loadFrameScript(CHILD_LOGGER_SCRIPT, true);
// //   mm.loadFrameScript(CHILD_SCRIPT_API, true);
// //   mm.loadFrameScript(CHILD_SCRIPT, true);
// //   var src = iframe.src;
// //   iframe.src = "about:blank";
// //   window.setTimeout(function() {
// //     iframe.src = src;
// //   });
// // });
