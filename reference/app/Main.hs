module Main (main) where

import qualified Data.ByteString as BS
import Scoring (calculateMetrics, decodeRows, formatSummary, scoreRows)
import System.Environment (getArgs)
import System.Exit (exitFailure)

main :: IO ()
main = do
  arguments <- getArgs
  case arguments of
    [inputPath] -> run inputPath
    _ -> failWith "usage: reference-scorer <reviewed-cases.jsonl>"

run :: FilePath -> IO ()
run inputPath = do
  source <- BS.readFile inputPath
  case decodeRows source >>= scoreRows of
    Left message -> failWith message
    Right counts -> putStr (formatSummary (calculateMetrics counts))

failWith :: String -> IO a
failWith message = do
  putStrLn ("reference-scorer: " ++ message)
  exitFailure
