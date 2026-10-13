(
var source = thisProcess.nowExecutingPath.dirname.dirname.dirname +/+ "HelpSource/Classes";
var names = (source +/+ "*.schelp").pathMatch.collect { |path| PathName(path).fileNameWithoutExtension };
var pages = names.collect { |name| "Classes/" ++ name } ++ ["Guides/Stochastica", "Guides/Stochastica-GENDY"];
SCDoc.verbosity = 2;
SCDoc.indexAllDocuments(true);
pages.do { |path|
    var doc = SCDoc.documents[path];
    var tree;
    if(doc.isNil) { Error("Missing help page: " ++ path).throw };
    tree = SCDoc.parseDoc(doc);
    if(tree.isNil) { Error("Could not parse: " ++ path).throw };
    SCDoc.renderer.renderToFile(doc.destPath, doc, tree);
    ("RENDERED " ++ path).postln;
};
"GENDY_SCDOC_OK".postln;
0.exit;
)
