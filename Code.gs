function doPost(e) { 
  var GS = SpreadsheetApp.openById('SpreadsheetID')
  var SheetDate = new Date().toLocaleDateString();
  // Create a sheet for today if it doesn't exist and add column headers
  if (!GS.getSheetByName(SheetDate)) GS.insertSheet(SheetDate).getRange('A1:E1').setValues([['time','latitude','longitude','altitude','desc']]);
  var ThisSheet = GS.getSheetByName(SheetDate);
  // Row place holder
  var ThisRecord = [];
  // Timestamp
  ThisRecord[0] = new Date().toLocaleTimeString();
  // Get all contents
  var json = JSON.parse(e.postData.contents);
  // Decoded data
  ThisRecord[1]=json.decoded.payload[0].value.latitude;
  ThisRecord[2]=json.decoded.payload[0].value.longitude;
  ThisRecord[3]=json.decoded.payload[0].value.altitude;
  var HotspotNumber=-1;
  var Hotspots=json.hotspots;
  // Get all hotspots
  while (Hotspots[++HotspotNumber]) ThisRecord[4]=ThisRecord[4]+Hotspots[HotspotNumber].name+',';
  // Remove "undefined" and last comma
  ThisRecord[4]=ThisRecord[4].substring(9, ThisRecord[4].length - 1)
  // Save in spreadsheet
  ThisSheet.getRange(ThisSheet.getLastRow() + 1, 1, 1, ThisRecord.length).setValues([ThisRecord]);
}