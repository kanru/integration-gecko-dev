/* -*- Mode: javascript; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is OEone Calendar Code, released October 31st, 2001.
 *
 * The Initial Developer of the Original Code is
 * OEone Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Garth Smedley <garths@oeone.com>
 *                 Mike Potter <mikep@oeone.com>
 *                 Colin Phillips <colinp@oeone.com>
 *                 Karl Guertin <grayrest@grayrest.com> 
 *                 Mike Norton <xor@ivwnet.com>
 *                 ArentJan Banck <ajbanck@planet.nl> 
 *                 Eric Belhaire <belhaire@ief.u-psud.fr>
 *                 Matthew Willis <mattwillis@gmail.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

/***** calendar
* AUTHOR
*   Garth Smedley
*
* NOTES
*   Code for the calendar.
*
*   What is in this file:
*     - Global variables and functions - Called directly from the XUL
*     - Several classes:
*  
* IMPLEMENTATION NOTES 
*
**********
*/






/*-----------------------------------------------------------------
*  G L O B A L     V A R I A B L E S
*/

// single global instance of CalendarWindow
var gCalendarWindow;

var prefService = Components.classes["@mozilla.org/preferences-service;1"]
                            .getService(Components.interfaces.nsIPrefService);
var rootPrefNode = prefService.getBranch(null); // preferences root node


/*-----------------------------------------------------------------
*  G L O B A L     C A L E N D A R      F U N C T I O N S
*/

/** 
* Called from calendar.xul window onload.
*/

function calendarInit() 
{
   // set up the CalendarWindow instance
   
   gCalendarWindow = new CalendarWindow();

   // Set up the checkbox variables.  Do not use the typical change*() functions
   // because those will actually toggle the current value.
   if (document.getElementById("toggle_workdays_only").getAttribute("checked")
       == 'true') {
       var deck = document.getElementById("view-deck")
       for each (view in deck.childNodes) {
           view.workdaysOnly = true;
       }
       deck.selectedPanel.goToDay(deck.selectedPanel.selectedDay);
   }

   // tasksInView is true by default
   if (document.getElementById("toggle_tasks_in_view").getAttribute("checked")
       != 'true') {
       var deck = document.getElementById("view-deck")
       for each (view in deck.childNodes) {
           view.tasksInView = false;
       }
       deck.selectedPanel.goToDay(deck.selectedPanel.selectedDay);
   }

   // set up the unifinder
   
   prepareCalendarUnifinder();

   prepareCalendarToDoUnifinder();
   
   update_date();

   checkForMailNews();

   initCalendarManager();

   if (("arguments" in window) && (window.arguments.length) &&
       (typeof(window.arguments[0]) == "object") &&
       ("channel" in window.arguments[0]) ) {
       gCalendarWindow.calendarManager.checkCalendarURL( 
           window.arguments[0].channel );
   }

   // a bit of a hack since the menulist doesn't remember the selected value
   var value = document.getElementById( 'event-filter-menulist' ).value;
   document.getElementById( 'event-filter-menulist' ).selectedItem = 
       document.getElementById( 'event-filter-'+value );

   var toolbox = document.getElementById("calendar-toolbox");
   toolbox.customizeDone = CalendarToolboxCustomizeDone;
}

// Set the date and time on the clock and set up a timeout to refresh the clock when the 
// next minute ticks over

function update_date()
{
   // get the current time
   var now = new Date();
   
   var tomorrow = new Date( now.getFullYear(), now.getMonth(), ( now.getDate() + 1 ) );
   
   var milliSecsTillTomorrow = tomorrow.getTime() - now.getTime();
   
   gCalendarWindow.currentView.hiliteTodaysDate();

   refreshEventTree();

   // Is an nsITimer/callback extreme overkill here? Yes, but it's necessary to
   // workaround bug 291386.  If we don't, we stand a decent chance of getting
   // stuck in an infinite loop.
   var udCallback = {
       notify: function(timer) {
           update_date();
       }
   };

   var timer = Components.classes["@mozilla.org/timer;1"]
                         .createInstance(Components.interfaces.nsITimer);
   timer.initWithCallback(udCallback, milliSecsTillTomorrow, timer.TYPE_ONE_SHOT);
}

/** 
* Called from calendar.xul window onunload.
*/

function calendarFinish()
{
   // Workaround to make the selected tab persist. See bug 249552.
   var tabbox = document.getElementById("tablist");
   tabbox.setAttribute("selectedIndex", tabbox.selectedIndex);

   finishCalendarUnifinder();
   
   finishCalendarToDoUnifinder();

   finishCalendarManager();
}

function launchPreferences()
{
    var applicationName = navigator.vendor;
    if (applicationName == "Mozilla" || applicationName == "Firebird" || applicationName == "")
        goPreferences( "calendarPanel", "chrome://calendar/content/pref/calendarPref.xul", "calendarPanel" );
    else
        window.openDialog("chrome://calendar/content/pref/prefBird.xul", "PrefWindow", "chrome,titlebar,resizable,modal");
}

/** 
* Called when the new event button is clicked
*/
var gNewDateVariable = null;

function newEventCommand( event )
{
   newEvent();
}


/** 
* Called when the new task button is clicked
*/

function newToDoCommand()
{
  newToDo( null, null ); // new task button defaults to undated todo
}

function newCalendarDialog()
{
    openCalendarWizard();
}

function editCalendarDialog(event)
{
    openCalendarProperties(document.popupNode.calendar, null);
}

function calendarListboxDoubleClick(event) {
    if(event.target.calendar)
        openCalendarProperties(event.target.calendar, null);
    else
        openCalendarWizard();
}

function checkCalListTarget() {
    if(!document.popupNode.calendar) {
        document.getElementById("calpopup-edit").setAttribute("disabled", "true");
        document.getElementById("calpopup-delete").setAttribute("disabled", "true");
        document.getElementById("calpopup-publish").setAttribute("disabled", "true");
    }
    else {
        document.getElementById("calpopup-edit").removeAttribute("disabled");
        document.getElementById("calpopup-delete").removeAttribute("disabled");
        document.getElementById("calpopup-publish").removeAttribute("disabled");
    }
}

function deleteCalendar(event)
{
    var cal = document.popupNode.calendar
    getDisplayComposite().removeCalendar(cal.uri);
    var calMgr = getCalendarManager();
    calMgr.unregisterCalendar(cal);
    // Delete file?
    //calMgr.deleteCalendar(cal);
}

/** 
* Defaults null start/end date based on selected date in current view.
* Defaults calendarFile to the selected calendar file.
* Calls editNewEvent. 
*/

function newEvent(startDate, endDate, allDay)
{
   // create a new event to be edited and added
   var calendarEvent = createEvent();

   if (!startDate) {
       startDate = gCalendarWindow.currentView.getNewEventDate();
   }

   calendarEvent.startDate.jsDate = startDate;

   if (!endDate) {
       var MinutesToAddOn = getIntPref(gCalendarWindow.calendarPreferences.calendarPref, "event.defaultlength", gCalendarBundle.getString("defaultEventLength" ) );
       
       endDate = new Date(startDate);
       endDate.setMinutes(endDate.getMinutes() + MinutesToAddOn);
   }

   calendarEvent.endDate.jsDate = endDate

   var prefService = Components.classes["@mozilla.org/preferences-service;1"]
                               .getService(Components.interfaces.nsIPrefService);
   var alarmsBranch = prefService.getBranch("calendar.alarms.");

   if (alarmsBranch.getIntPref("onforevents") == 1) {
       // alarmTime doesn't matter, it just can't be null
       calendarEvent.alarmTime = createDateTime();

       calendarEvent.setProperty("alarmUnits", alarmsBranch.getCharPref("eventalarmunit"));
       calendarEvent.setProperty("alarmLength", alarmsBranch.getIntPref("eventalarmlen"));
       calendarEvent.setProperty("alarmRelation", "START");
   }

   if (allDay)
       calendarEvent.startDate.isDate = true;

   var calendar = getSelectedCalendarOrNull();

   editNewEvent( calendarEvent, calendar );
}

/*
* Defaults null start/due date to the no_date date.
* Defaults calendarFile to the selected calendar file.
* Calls editNewToDo.
*/
function newToDo ( startDate, dueDate ) 
{
    var calendarToDo = createToDo();
   
    // created todo has no start or due date unless user wants one
    if (startDate) 
        calendarToDo.entryDate = jsDateToDateTime(startDate);

    if (dueDate)
        calendarToDo.dueDate = jsDateToDateTime(startDate);

   var prefService = Components.classes["@mozilla.org/preferences-service;1"]
                               .getService(Components.interfaces.nsIPrefService);
   var alarmsBranch = prefService.getBranch("calendar.alarms.");

   if (alarmsBranch.getIntPref("onfortodos") == 1) {
       // alarmTime doesn't matter, it just can't be null
       calendarToDo.alarmTime = createDateTime();

       // You can't have an alarm if the entryDate doesn't exist.
       if (!calendarToDo.entryDate)
           calendarToDo.entryDate = jsDateToDateTime(
                                    gCalendarWindow.currentView.getNewEventDate());

       calendarToDo.setProperty("alarmUnits", alarmsBranch.getCharPref("todoalarmunit"));
       calendarToDo.setProperty("alarmLength", alarmsBranch.getIntPref("todoalarmlen"));
       calendarToDo.setProperty("alarmRelation", "START");
   }

    var calendar = getSelectedCalendarOrNull();
    
    editNewToDo(calendarToDo, calendar);
}

/**
 * Get the default calendar selected in the calendars tab.
 * Returns a calICalendar object, or null if none selected.
 */
function getSelectedCalendarOrNull()
{
   var selectedCalendarItem = document.getElementById( "list-calendars-listbox" ).selectedItem;
   
   if ( selectedCalendarItem )
     return selectedCalendarItem.calendar;
   else
     return null;
}

/**
* Launch the event dialog to edit a new (created, imported, or pasted) event.
* 'calendar' is a calICalendar object.
* When the user clicks OK "addEventDialogResponse" is called
*/

function editNewEvent( calendarEvent, calendar )
{
  openEventDialog(calendarEvent,
                  "new",
                  self.addEventDialogResponse,
                  calendar);
}

/**
* Launch the todo dialog to edit a new (created, imported, or pasted) ToDo.
* 'calendar' is a calICalendar object.
* When the user clicks OK "addToDoDialogResponse" is called
*/
function editNewToDo( calendarToDo, calendar )
{
  openEventDialog(calendarToDo,
                  "new",
                  self.addToDoDialogResponse,
                  calendar);
}

/** 
* Called when the user clicks OK in the new event dialog
* 'calendar' is a calICalendar object.
*
* Updates the data source.  The unifinder views and the calendar views will be
* notified of the change through their respective observers.
*/

function addEventDialogResponse( calendarEvent, calendar )
{
   saveItem( calendarEvent, calendar, "addEvent" );
}


/** 
* Called when the user clicks OK in the new to do item dialog
* 'calendar' is a calICalendar object.
*/

function addToDoDialogResponse( calendarToDo, calendar )
{
    addEventDialogResponse(calendarToDo, calendar);
}


/** 
* Helper function to launch the event dialog to edit an event.
* When the user clicks OK "modifyEventDialogResponse" is called
*/

function editEvent( calendarEvent )
{
  openEventDialog(calendarEvent,
                  "edit",
                  self.modifyEventDialogResponse,
                  null);
}

function editToDo( calendarTodo )
{
  openEventDialog(calendarTodo,
                  "edit",
                  self.modifyEventDialogResponse,
                  null);
}
   
/** 
* Called when the user clicks OK in the edit event dialog
* 'calendar' is a calICalendar object.
*
* Update the data source, the unifinder views and the calendar views will be
* notified of the change through their respective observers
*/

function modifyEventDialogResponse( calendarEvent, calendar, originalEvent )
{
   saveItem( calendarEvent, calendar, "modifyEvent", originalEvent );
}


/** 
* Called when the user clicks OK in the edit event dialog
* 'calendar' is a calICalendar object.
*
* Update the data source, the unifinder views and the calendar views will be
* notified of the change through their respective observers
*/

function modifyToDoDialogResponse( calendarToDo, calendar, originalToDo )
{
    modifyEventDialogResponse(calendarToDo, calendar, originalToDo);
}


/** PRIVATE: open event dialog in mode, and call onOk if ok is clicked.
    'mode' is "new" or "edit".
    'calendar' is default calICalendar, typically from getSelectedCalendarOrNull
 **/
function openEventDialog(calendarEvent, mode, onOk, calendar)
{
  // set up a bunch of args to pass to the dialog
  var args = new Object();
  args.calendarEvent = calendarEvent;
  args.mode = mode;
  args.onOk = onOk;

  if( calendar )
    args.calendar = calendar;

  // wait cursor will revert to auto in eventDialog.js loadCalendarEventDialog
  window.setCursor( "wait" );
  // open the dialog modally
  openDialog("chrome://calendar/content/eventDialog.xul", "caEditEvent", "chrome,titlebar,modal", args );
}

/**
*  This is called from the unifinder's edit command
*/

function editEventCommand()
{
   if( gCalendarWindow.EventSelection.selectedEvents.length == 1 )
   {
      var calendarEvent = gCalendarWindow.EventSelection.selectedEvents[0];

      if( calendarEvent != null )
      {
         editEvent( calendarEvent.parentItem );
      }
   }
}


//originalEvent is the item before edits were committed, 
//used to check if there were external changes for shared calendar
function saveItem( calendarEvent, calendar, functionToRun, originalEvent )
{
    dump(functionToRun + " " + calendarEvent.title + "\n");

    if (functionToRun == 'addEvent') {
        doTransaction('add', calendarEvent, calendar, null, null);
    } else if (functionToRun == 'modifyEvent') {
        // compare cal.uri because there may be multiple instances of
        // calICalendar or uri for the same spec, and those instances are
        // not ==.
        if (!originalEvent.calendar || 
            (originalEvent.calendar.uri.equals(calendar.uri)))
            doTransaction('modify', calendarEvent, calendar, originalEvent, null);
        else
            doTransaction('move', calendarEvent, calendar, originalEvent, null);
    }
}


/**
*  This is called from the unifinder's delete command
*
*/
function deleteItems( SelectedItems, DoNotConfirm )
{
    if (!SelectedItems)
        return;

    startBatchTransaction();
    for (i in SelectedItems) {
        doTransaction('delete', SelectedItems[i], SelectedItems[i].calendar, null, null);
    }
    endBatchTransaction();
}


/**
*  Delete the current selected item with focus from the ToDo unifinder list
*/
function deleteEventCommand( DoNotConfirm )
{
   var SelectedItems = gCalendarWindow.EventSelection.selectedEvents;
   deleteItems( SelectedItems, DoNotConfirm );
   gCalendarWindow.EventSelection.emptySelection();
}


/**
*  Delete the current selected item with focus from the ToDo unifinder list
*/
function deleteToDoCommand( DoNotConfirm )
{
   var SelectedItems = new Array();
   var tree = document.getElementById( ToDoUnifinderTreeName );
   var start = new Object();
   var end = new Object();
   var numRanges = tree.view.selection.getRangeCount();
   var t;
   var v;
   var toDoItem;
   for (t = 0; t < numRanges; t++) {
      tree.view.selection.getRangeAt(t, start, end);
      for (v = start.value; v <= end.value; v++) {
         toDoItem = tree.taskView.getCalendarTaskAtRow( v );
         SelectedItems.push( toDoItem );
      }
   }
   deleteItems( SelectedItems, DoNotConfirm );
   tree.view.selection.clearSelection();
}


function goFindNewCalendars()
{
   //launch the browser to http://www.apple.com/ical/library/
   var browserService = penapplication.getService( "org.penzilla.browser" );
   if(browserService)
   {
       browserService.setUrl("http://www.icalshare.com/");
       browserService.focusBrowser();
   }
}

function selectAllEvents()
{
    //XXX
    throw "Broken by the switch to the new views"; 
}

function closeCalendar()
{
   self.close();
}

function print()
{
    window.openDialog("chrome://calendar/content/printDialog.xul",
                      "printdialog","chrome");
}

function getCharPref (prefObj, prefName, defaultValue)
{
    try {
        return prefObj.getCharPref (prefName);
    } catch (e) {
        prefObj.setCharPref( prefName, defaultValue );  
        return defaultValue;
    }
}

function getIntPref(prefObj, prefName, defaultValue)
{
    try {
        return prefObj.getIntPref(prefName);
    } catch (e) {
        prefObj.setIntPref(prefName, defaultValue);  
        return defaultValue;
    }
}

function getBoolPref (prefObj, prefName, defaultValue)
{
    try
    {
        return prefObj.getBoolPref (prefName);
    }
    catch (e)
    {
       prefObj.setBoolPref( prefName, defaultValue );  
       return defaultValue;
    }
}

function GetUnicharPref(prefObj, prefName, defaultValue)
{
    try {
      return prefObj.getComplexValue(prefName, Components.interfaces.nsISupportsString).data;
    }
    catch(e)
    {
      SetUnicharPref(prefObj, prefName, defaultValue);
        return defaultValue;
    }
}

function SetUnicharPref(aPrefObj, aPrefName, aPrefValue)
{
    try {
      var str = Components.classes["@mozilla.org/supports-string;1"]
                          .createInstance(Components.interfaces.nsISupportsString);
      str.data = aPrefValue;
      aPrefObj.setComplexValue(aPrefName, Components.interfaces.nsISupportsString, str);
    }
    catch(e) {}
}

/* Change the only-workday checkbox */
function changeOnlyWorkdayCheckbox() {
    var oldValue = (document.getElementById("toggle_workdays_only")
                           .getAttribute("checked") == 'true');
    document.getElementById("toggle_workdays_only")
            .setAttribute("checked", !oldValue);

    // This does NOT make the views refresh themselves
    for each (view in document.getElementById("view-deck").childNodes) {
        view.workdaysOnly = !oldValue;
    }

    // No point in updating views that aren't shown.  They'll notice the change
    // next time we call their goToDay function, just update the current view
    var currentView = document.getElementById("view-deck").selectedPanel;
    currentView.goToDay(currentView.selectedDay);
}

/* Change the display-todo-inview checkbox */
function changeDisplayToDoInViewCheckbox() {
    var oldValue = (document.getElementById("toggle_tasks_in_view")
                           .getAttribute("checked") == 'true');
    document.getElementById("toggle_tasks_in_view")
            .setAttribute("checked", !oldValue);

    // This does NOT make the views refresh themselves
    for each (view in document.getElementById("view-deck").childNodes) {
        view.tasksInView = !oldValue;
    }

    // No point in updating views that aren't shown.  They'll notice the change
    // next time we call their goToDay function, just update the current view
    var currentView = document.getElementById("view-deck").selectedPanel;
    currentView.goToDay(currentView.selectedDay);
}

// about Calendar dialog
function displayCalendarVersion()
{
  // uses iframe, but iframe does not auto-size per bug 80713, so provide height
  window.openDialog("chrome://calendar/content/about.xul", "About","modal,centerscreen,chrome,width=500,height=450,resizable=yes");
}


// about Sunbird dialog
function openAboutDialog()
{
  window.openDialog("chrome://calendar/content/aboutDialog.xul", "About", "modal,centerscreen,chrome,resizable=no");
}

function openPreferences()
{
  openDialog("chrome://calendar/content/pref/pref.xul","PrefWindow",
             "chrome,titlebar,resizable,modal");
}

// Next two functions make the password manager menu option
// only show up if there is a wallet component. Assume that
// the existence of a wallet component means wallet UI is there too.
function checkWallet()
{
  if ('@mozilla.org/wallet/wallet-service;1' in Components.classes) {
    document.getElementById("password-manager-menu")
            .removeAttribute("hidden");
  }
}

function openWalletPasswordDialog()
{
  window.openDialog("chrome://communicator/content/wallet/SignonViewer.xul",
                    "_blank","chrome,resizable=yes","S");
}

var strBundleService = null;
function srGetStrBundle(path)
{
  var strBundle = null;

  if (!strBundleService) {
      try {
          strBundleService =
              Components.classes["@mozilla.org/intl/stringbundle;1"].getService();
          strBundleService =
              strBundleService.QueryInterface(Components.interfaces.nsIStringBundleService);
      } catch (ex) {
          dump("\n--** strBundleService failed: " + ex + "\n");
          return null;
      }
  }

  strBundle = strBundleService.createBundle(path);
  if (!strBundle) {
        dump("\n--** strBundle createInstance failed **--\n");
  }
  return strBundle;
}

function CalendarCustomizeToolbar()
{
  // Disable the toolbar context menu items
  var menubar = document.getElementById("main-menubar");
  for (var i = 0; i < menubar.childNodes.length; ++i)
    menubar.childNodes[i].setAttribute("disabled", true);
    
  var cmd = document.getElementById("cmd_CustomizeToolbars");
  cmd.setAttribute("disabled", "true");
  
  window.openDialog("chrome://calendar/content/customizeToolbar.xul", "CustomizeToolbar",
                    "chrome,all,dependent", document.getElementById("calendar-toolbox"));
}

function CalendarToolboxCustomizeDone(aToolboxChanged)
{
  // Re-enable parts of the UI we disabled during the dialog
  var menubar = document.getElementById("main-menubar");
  for (var i = 0; i < menubar.childNodes.length; ++i)
    menubar.childNodes[i].setAttribute("disabled", false);
  var cmd = document.getElementById("cmd_CustomizeToolbars");
  cmd.removeAttribute("disabled");

  // XXX Shouldn't have to do this, but I do
  window.focus();
}

var gTransactionMgr = Components.classes["@mozilla.org/transactionmanager;1"]
                                .createInstance(Components.interfaces.nsITransactionManager);
function doTransaction(aAction, aItem, aCalendar, aOldItem, aListener) {
    var txn = new calTransaction(aAction, aItem, aCalendar, aOldItem, aListener);
    gTransactionMgr.doTransaction(txn);
    updateUndoRedoMenu();
}

function undo() {
    gTransactionMgr.undoTransaction();
    updateUndoRedoMenu();
}

function redo() {
    gTransactionMgr.redoTransaction();
    updateUndoRedoMenu();
}

function startBatchTransaction() {
    gTransactionMgr.beginBatch();
}
function endBatchTransaction() {
    gTransactionMgr.endBatch();
    updateUndoRedoMenu();
}

function canUndo() {
    return (gTransactionMgr.numberOfUndoItems > 0);
}
function canRedo() {
    return (gTransactionMgr.numberOfRedoItems > 0);
}

function updateUndoRedoMenu() {
    if (gTransactionMgr.numberOfUndoItems)
        document.getElementById('undo_command').removeAttribute('disabled');
    else    
        document.getElementById('undo_command').setAttribute('disabled', true);

    if (gTransactionMgr.numberOfRedoItems)
        document.getElementById('redo_command').removeAttribute('disabled');
    else    
        document.getElementById('redo_command').setAttribute('disabled', true);
}

// Valid values for aAction: 'add', 'modify', 'delete', 'move'
// aOldItem is only needed for aAction == 'modify'
function calTransaction(aAction, aItem, aCalendar, aOldItem, aListener) {
    this.mAction = aAction;
    this.mItem = aItem;
    this.mCalendar = aCalendar;
    this.mOldItem = aOldItem;
    this.mListener = aListener;
}

calTransaction.prototype = {
    mAction: null,
    mItem: null,
    mCalendar: null,
    mOldItem: null,
    mOldCalendar: null,
    mListener: null,

    QueryInterface: function (aIID) {
        if (!aIID.equals(Components.interfaces.nsISupports) &&
            !aIID.equals(Components.interfaces.nsITransaction))
        {
            throw Components.results.NS_ERROR_NO_INTERFACE;
        }
        return this;
    },

    doTransaction: function () {
        switch (this.mAction) {
            case 'add':
                this.mCalendar.addItem(this.mItem, this.mListener);
                break;
            case 'modify':
                this.mCalendar.modifyItem(this.mItem, this.mOldItem,
                                          this.mListener);
                break;
            case 'delete':
                this.mCalendar.deleteItem(this.mItem, this.mListener);
                break;
            case 'move':
                this.mOldCalendar = this.mOldItem.calendar;
                this.mOldCalendar.deleteItem(this.mOldItem, this.mListener);
                this.mCalendar.addItem(this.mItem, this.mListener);
                break;
        }
    },
    undoTransaction: function () {
        switch (this.mAction) {
            case 'add':
                this.mCalendar.deleteItem(this.mItem, null);
                break;
            case 'modify':
                this.mCalendar.modifyItem(this.mOldItem, this.mItem, null);
                break;
            case 'delete':
                this.mCalendar.addItem(this.mItem, null);
                break;
            case 'move':
                this.mCalendar.deleteItem(this.mItem, this.mListener);
                this.mOldCalendar.addItem(this.mOldItem, this.mListener);
                break;
        }
    },
    redoTransaction: function () {
        this.doTransaction();
    },
    isTransient: false,
    
    merge: function (aTransaction) {
        // No support for merging
        return false;
    }
}
