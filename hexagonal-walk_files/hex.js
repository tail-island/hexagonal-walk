/**
 * 描画は http://tmlife.net/programming/javascript/html5-canvas-polygon-draw.html を参考
 */
 
(function(np){
    
    np = np || window;
    
    /**
     * ポリゴンパスセット
     * @param   {CanvasRenderingContext2D}  ctx 描画先コンテキスト
     * @param   {Number}    x 値
     * @param   {Number}    y 値
     * @param   {Number}    radius  半径
     */
    np.hex = function(ctx, x, y, radius)
    {
        // 内角
        var radDiv = (Math.PI*2)/6;
        // 回転オフセット(270°)
        var radOffset = -Math.PI/2;
        
        // パス描画
        ctx.moveTo(x + Math.cos(radOffset)*radius, y + Math.sin(radOffset)*radius);
        for (var i=1; i<6; ++i)
        {
            var rad = radDiv*i+radOffset;
            ctx.lineTo(
                x + Math.cos(rad)*radius,
                y + Math.sin(rad)*radius
            );
        }
        ctx.closePath();
    };
    
    /**
     * ポリゴンパス塗りつぶし描画
     * @param   {CanvasRenderingContext2D}  ctx 描画先コンテキスト
     * @param   {Number}    x 値
     * @param   {Number}    y 値
     * @param   {Number}    radius  半径
     */
    np.fillHex = function(ctx, x, y, radius)
    {
        ctx.beginPath();
        hex(ctx, x, y, radius);
        ctx.fill();
    };
    
    /**
     * ポリゴンパスストローク描画
     * @param   {CanvasRenderingContext2D}  ctx 描画先コンテキスト
     * @param   {Number}    x 値
     * @param   {Number}    y 値
     * @param   {Number}    radius  半径
     */
    np.strokeHex = function(ctx, x, y, radius)
    {
        ctx.beginPath();
        hex(ctx, x, y, radius);
        ctx.stroke();
    };
    
    /**
     * へクス内数字描画
     * @param   {CanvasRenderingContext2D}  ctx 描画先コンテキスト
     * @param   {Number}    x 値
     * @param   {Number}    y 値
     * @param   {Number}    str  描画する数値
     * @param   {Number}    radius  へクス半径
     */
    np.fillHexText = function(ctx, x, y, str, radius)
    {
        fSize = 20;
        if (radius < 17)
        {
            fSize = radius * 1.2;
        }
        
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.font = fSize + "px sans-serif";
        
        //スタート
        if (str == 0)
        {
            str = "S";
        }
        ctx.fillText(str, x, y);
    };
    
    /**
     * へクス隣接判定
     * @param   {Number}    x1 へクス1x座標
     * @param   {Number}    y1 へクス1y座標
     * @param   {Number}    x2 へクス2x座標
     * @param   {Number}    y2 へクス2y座標
     */
    np.isNextHex = function(x1, y1, x2, y2)
    {
        //x軸方向への移動
        if (y1 == y2)
        {
           if (Math.abs(x1 - x2) == 1)
           {
               return true;
           }else{
               return false;
           }
        }
        //y軸方向への移動
        if (x1 == x2)
        {
           if (Math.abs(y1 - y2) == 1)
           {
               return true;
           }else{
               return false;
           }
        }
        //z軸(第三軸）方向への移動
        if (x1 - x2 == 1)
        {
           if (y1 - y2 == -1)
           {
              return true;
           }else{
              return false;
           }
        }else if (x1 - x2 == -1){
           if (y1 - y2 == 1)
           {
              return true;
           }else{
              return false;
           }
        }
        
        return false;
    }
    
})();
