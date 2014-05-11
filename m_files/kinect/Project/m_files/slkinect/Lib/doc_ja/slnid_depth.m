%% NID Depth
%
% Kinectデバイスから取得した深度データを返します。
%
% Copyright 2011-2012 The MathWorks, Inc.

%% 入力信号
%
% * SYNC (uint32): IMAQブロックから出力される同期用時間信号入力 [msec]。

%% 出力信号
%
% * Depth (double): 各ピクセル毎の深度データ [m]。深度データ = 0は、深度データの計測不可を意味します。
%
% * XYZ (double): XYZ座標データ。座標は、Real world/Projectiveのいずれかに設定できます。
%
% * X/Y/Z (double): X, Y, Zが独立した座標データ [m]。座標は、Real world/Projectiveのいずれかに設定できます。
%
% * Image (uint8): 深度データのRGB24イメージ。黒色は深度データの計測不可を意味します。

%% ブロックパラメータ
%
% * Output: 深度データの出力モード設定。
%
% * Real world coordinates (Default: Projective): Real world (X, Y, Z [m]) または、Projective (X,Y [pixel], Z [m])座標を選択。XYZおよびX/Y/Z深度データ出力モードにのみ有効。
%
% Projective XYZ座標:
%
% <<ProjectiveXYZ.png>>
%
% Real world XYZ座標:
%
% <<RealWorldXYZ.png>>
%
% * Image output: 深度データのRGB24イメージ出力を設定。
