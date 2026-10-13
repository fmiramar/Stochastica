Lorenz96 : MultiOutUGen {
    *ar { |numCells=8, forcing=8, timeScale=1, substeps=1, initialBuf = -1, reload=0, reset=0|
        ^this.multiNew('audio',numCells.asInteger.clip(4,64),forcing,timeScale,substeps,initialBuf,reload,reset)
    }
    *kr { |numCells=8, forcing=8, timeScale=1, substeps=1, initialBuf = -1, reload=0, reset=0|
        ^this.multiNew('control',numCells.asInteger.clip(4,64),forcing,timeScale,substeps,initialBuf,reload,reset)
    }
    init { |numCells ... inputs| this.inputs=inputs; ^this.initOutputs(numCells,rate) }
    argNamesInputsOffset { ^2 }
}
