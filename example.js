var ok = loadPluginLib('/opt/vmbase/extensions/', 'libadsbPlugin');
if( ok == false ) {
    print('cannot load lib, stop.');
    exit();
}


var ok = ImportObject('libadsbPlugin', 'ADSB');
if( ok == false ) {
    print('Cannot load object');
    exit();
}

var adsb = new ADSB();
if( adsb.start() == false ) {
    print('Could not start ADSB');
    exit();
}

for( ;; ) {
    var msg = MBoxPopWait('ADSBMESSAGES', 1000 ) ;
    
    print( JSON.stringify(msg));
}
