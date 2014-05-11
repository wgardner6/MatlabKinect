%% NID IR
%
% Kinectデバイスから取得したIR(赤外線)データを返します。
% IRブロックは、IMAQブロック、Depthブロックとのみ併用可能です。
%
% Copyright 2011-2012 The MathWorks, Inc.

%% 入力信号
%
% * SYNC (uint32): IMAQブロックから出力される同期用時間信号入力 [msec]。

%% 出力信号
%
% * IR (uint16): 各ピクセル毎のIRデータ。
%
% * Image (uint8): IRデータのRGB24イメージ(グレイスケール)。

%% ブロックパラメータ
%
% * Image output: IRデータのRGB24イメージ出力を設定。
