%% NID Motion
%
% Kinectデバイスから取得したモーション認識IDデータを返します。
%
% Copyright 2011-2012 The MathWorks, Inc.

%% 入力信号
%
% * SYNC (uint32): IMAQブロックから出力される同期用時間信号入力 [msec]。

%% 出力信号
%
% * Motion (uint16): 各ピクセル毎のモーション認識IDデータ(0 to 15)。 ID = 0はモーション認識無しを意味します。
%
% * Image (uint8): モーション認識IDデータのRGB24イメージ。
% オブジェクトIDに次の各色が割り当てられています。
% Red = 1, Green = 2, Blue = 3, Yellow = 4, Cyan = 5, Magenda = 6, Orange = 7,
% Dark green = 8, Teal = 9, Purple = 10, Olive = 11, Maroon = 12, Silver = 13,
% Navy = 14, Gold = 15.

%% ブロックパラメータ
%
% * Image output: モーション認識IDデータのRGB24イメージ出力を設定。
