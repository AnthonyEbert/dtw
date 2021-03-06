\name{warp}
\alias{warp}
\title{Apply a warping to a given timeseries}
\description{

  Returns the indexing required to apply the optimal warping curve to a
  given timeseries (warps either into a query or into a reference).


}
\usage{
warp(d,index.reference=FALSE)
}
\arguments{
  \item{d}{\code{dtw} object specifying the warping curve to apply}

  \item{index.reference}{\code{TRUE} to warp a reference, \code{FALSE}
to warp a query}
}
\details{

  The warping is returned as a set of indices, which can be used to
  subscript the timeseries to be warped (or rows in a matrix, if one
  wants to warp a multivariate time series).  In other words,
  \code{warp} converts the warping curve, or its inverse, into a
  function in the explicit form.

  Multiple indices that would be mapped to a single point are averaged,
  with a warning. Gaps in the index sequence are filled by linear
  interpolation.

}

\value{
  A list of indices to subscript the timeseries. 
  }

  
\seealso{Examples in \code{\link{dtw}} show how to \emph{graphically}
  apply the warping via parametric   plots.
}

\examples{
idx<-seq(0,6.28,len=100);
query<-sin(idx)+runif(100)/10;
reference<-cos(idx)

alignment<-dtw(query,reference);


wq<-warp(alignment,index.reference=FALSE);
wt<-warp(alignment,index.reference=TRUE);

old.par <- par(no.readonly = TRUE);
par(mfrow=c(2,1));

plot(reference,main="Warping query");
  lines(query[wq],col="blue");

plot(query,type="l",col="blue",
  main="Warping reference");
  points(reference[wt]);

par(old.par);


##############
##
## Asymmetric step makes it "natural" to warp
## the reference, because every query index has
## exactly one image (q->t is a function)
##

alignment<-dtw(query,reference,step=asymmetric)
wt<-warp(alignment,index.reference=TRUE);

plot(query,type="l",col="blue",
  main="Warping reference, asymmetric step");
  points(reference[wt]);



}
\author{Toni Giorgino}
\keyword{ts}
