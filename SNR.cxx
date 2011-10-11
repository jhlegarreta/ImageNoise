#include "SNRCLP.h"
#include "itkImageFileReader.h"
#include "itkImage.h"
#include <itkImageRegionIteratorWithIndex.h>
#include <math.h>
#include <vector>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1


double Variance( std::vector< double > vec )
{
    double Av = 0 ;
    for(unsigned int i = 0 ; i < vec.size() ; i++ )
    {
      Av += vec[ i ] ;
    }
    Av /= (double)vec.size() ;
    double var = 0 ;
    for(unsigned int i = 0 ; i < vec.size() ; i++ )
    {
      var += ( vec[ i ] - Av ) * ( vec[ i ] - Av ) ;
    }
    var /= ( (double)vec.size() - 1.0 )  ;
    return var ;
}

int main( int argc, char * argv[] )
{
  PARSE_ARGS;
  if( insideRegion.empty() )
  {
    std::cerr << "You need to specify a signal (inside) region" << std::endl ;
    return EXIT_FAILURE ;
  }
  if( ( secondImage.empty() && outsideRegion.empty() ) 
     || ( !secondImage.empty() && !outsideRegion.empty() )
    )
  {
    std::cerr << "Specify either a second image or an outside region" << std::endl;
    return EXIT_FAILURE ;
  }
  typedef itk::Image< double , 3 > ImageType ;
  typedef itk::ImageFileReader< ImageType > ReaderType ;
  ReaderType::Pointer reader = ReaderType::New() ;
  reader->SetFileName( inputImage.c_str() ) ;
  reader->Update() ;
  typedef itk::Image< int , 3 > LabelType ;
  typedef itk::ImageFileReader< LabelType > LabelReaderType ;
  LabelReaderType::Pointer l1reader = LabelReaderType::New() ;
  l1reader->SetFileName( insideRegion.c_str() ) ;
  l1reader->Update() ;
  typedef ImageType::IndexType IndexType ;
  IndexType index ;
  typedef itk::ImageRegionIteratorWithIndex< LabelType > LabelIterator ;
  LabelIterator it1( l1reader->GetOutput() , l1reader->GetOutput()->GetLargestPossibleRegion() ) ;
  double S = 0 ;
  long count = 0 ;
  for( it1.GoToBegin() ; !it1.IsAtEnd() ; ++it1 )
  {
    if( it1.Get() )
    {
      index = it1.GetIndex() ;
      S += reader->GetOutput()->GetPixel( index ) ;
      count++ ;
    }
  }
  S /= (double)count ;

  if( !outsideRegion.empty() )
  {
    LabelReaderType::Pointer l2reader = LabelReaderType::New() ;
    l2reader->SetFileName( outsideRegion.c_str() ) ;
    l2reader->Update() ;
    LabelIterator it2( l2reader->GetOutput() , l2reader->GetOutput()->GetLargestPossibleRegion() ) ;
    std::vector< double > val ;
    for( it2.GoToBegin() ; !it2.IsAtEnd() ; ++it2 )
    {
      if( it2.Get() )
      {
        index = it2.GetIndex() ;
        val.push_back( reader->GetOutput()->GetPixel( index ) ) ;
      }
    }
    double var = Variance( val ) ;
    std::cout << 0.655*S/sqrt(var) << std::endl ;
  }
  else if( !secondImage.empty() )
  {
    ReaderType::Pointer reader2 = ReaderType::New() ;
    reader2->SetFileName( secondImage.c_str() ) ;
    reader2->Update() ;
    typedef itk::ImageRegionIteratorWithIndex< ImageType > ImageIterator ;
    ImageIterator imIt( reader->GetOutput() , reader->GetOutput()->GetLargestPossibleRegion() ) ;
    std::vector< double > vec ;
    for( imIt.GoToBegin() ; !imIt.IsAtEnd() ; ++imIt )
    {
      index = imIt.GetIndex() ;
      vec.push_back( imIt.Get() - reader2->GetOutput()->GetPixel( index ) ) ;
    }
    double var = Variance( vec ) ;
    std::cout << sqrt( 2.0 ) * S / sqrt(var) << std::endl ;
  }
  else
  {
    std::cerr << "Problems in the arguments" << std::endl ;
    return EXIT_FAILURE ;
  }
  return EXIT_SUCCESS ;
}
