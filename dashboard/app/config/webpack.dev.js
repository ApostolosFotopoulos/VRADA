const HtmlWebPackPlugin = require( 'html-webpack-plugin' );
const path = require( 'path' );
const webpack = require('webpack')

module.exports = {
  mode:'development',
  entry: [
    'webpack-dev-server/client?http://0.0.0.0:8080',
    './src/index.js',
  ],
  output: {
    path: path.resolve( __dirname, '../dist' ),
    filename:'main.chunk.js',
    publicPath: '/'
  },
  devServer:{
    publicPath: '/',
    hot:true,
    contentBase:'../dist'
  },
  module: {
    rules: [
      {
        test: /\.(js|jsx)$/,
        exclude: [path.resolve(__dirname, "../node_modules")],
        use: {
          loader: "babel-loader"
        }
      },
      {
        test: /\.css$/,
        use: ['style-loader', 'css-loader'],
      },
      {
        test: /\.(png|j?g|svg|gif)?$/,
        use: 'file-loader'
      },
      {
        test: /\.less$/,
        use: [
          {loader: "style-loader"},
          {loader: "css-loader"},
          {loader: "less-loader",
          }
        ]
    }
    ]
   },
  plugins: [
    new HtmlWebPackPlugin({
      template: path.resolve( __dirname, '../public/index.html' ),
      filename: 'index.html',
      inject:true
    }),
    new webpack.HotModuleReplacementPlugin(),
  ],
  optimization:{
    minimize:true
  }
};